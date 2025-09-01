#include <cstdint>
#include <vector>
#include <bit>
#include <algorithm>
#include <utility>
#include "board/board.h"
#include "move/move.h"
#include "move/precompute_moves.h"
#include "game/game.h"
#include "chess_types.h"
#include "engine/evaluation.h"
#include "engine/piece_tables.h"
#include "engine/precompute.h"

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::Bitboard;
using Chess::toIndex;

Move Evaluation::killerMoves[256][2];
int16_t Evaluation::historyHeuristics[2][6][64][64];

int16_t Evaluation::gamePhase(Board& board) {
    constexpr Piece pieces[4] = {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
    constexpr Colour colours[2] = {Colour::WHITE, Colour::BLACK};
    constexpr int phaseValues[6] = {0, 1, 1, 2, 4, 0};
    
    int totalPhase = 0;
    for (Colour colour : colours) {
        for (Piece piece : pieces) {
            Bitboard bitboard = board.getBitboard(piece, colour);
            totalPhase += std::popcount(bitboard) * phaseValues[toIndex(piece)];
        }
    }
    
    return std::min(totalPhase, Evaluation::MAX_PHASE);
}

int16_t Evaluation::pieceValueEvaluation(Board& board, Colour colour, int16_t phase) {
    constexpr Piece pieces[6] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN, Piece::KING};
    const Colour opposingColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    const uint8_t kingSquare = board.getKingSquare(colour);
    const uint8_t opposingKingSquare = board.getKingSquare(opposingColour);
    const uint8_t kingFile = Board::getFile(kingSquare);
    const uint8_t opposingKingFile = Board::getFile(opposingKingSquare);
    const uint8_t opposingKingRank = Board::getRank(opposingKingSquare);

    const Bitboard allPiecesBitboard = board.getPiecesBitboard();
    const Bitboard piecesBitboard = board.getBitboard(colour);
    const Bitboard pawnsBitboard = board.getBitboard(Piece::PAWN, colour);
    const Bitboard opposingPawnsBitboard = board.getBitboard(Piece::PAWN, opposingColour);
    const Bitboard allPawnsBitboard = pawnsBitboard | opposingPawnsBitboard;

    uint8_t c = toIndex(colour);
    uint8_t oc = toIndex(opposingColour);
    int32_t phasedEval = 0;
    int16_t eval = 0;

    for (uint8_t i = 0; i < 6; i++) {
        Bitboard bitboard = board.getBitboard(pieces[i], colour);
        while (bitboard) {
            // Piece evaluation
            eval += pieceEvals[i];

            // Piece Square Table evaluation
            uint8_t square = std::countr_zero(bitboard);
            if (colour == Colour::WHITE) square ^= 0x38; // Flip square from black to white's perspective
            phasedEval += PieceTables::tables[i][square] * phase + PieceTables::endgameTables[i][square] * (MAX_PHASE - phase);

            bitboard &= (bitboard - 1);
        }
    }

    // Pawn structure penalties
    for (uint8_t file = 0; file < 8; file++) {
        uint64_t mask = 0x0101010101010101ULL << file; // Mask of the current file
        uint8_t pawnCount = std::popcount(pawnsBitboard & mask);

        // Penalty for doubling pawns
        phasedEval += (pawnCount - 1) * (phase * DOUBLED_PAWN_PENALTY + (MAX_PHASE - phase) * DOUBLED_PAWN_PENALTY_END_GAME);

        // Penalty for isolated pawns
        uint64_t isolatedMask = EnginePrecompute::adjacentFileMaskTable[file];
        if (pawnsBitboard & isolatedMask) continue; // Contains pawn on either immediate left or right file
        phasedEval += pawnCount * (phase * ISOLATED_PAWN_PENALTY + (MAX_PHASE - phase) * ISOLATED_PAWN_PENALTY_END_GAME);
    }

    Bitboard pawnsBitboardTemp = pawnsBitboard;
    while (pawnsBitboardTemp) {
        uint8_t square = std::countr_zero(pawnsBitboardTemp);
        uint8_t nextSquare = (colour == Colour::WHITE) ? square + 8 : square - 8;
        uint64_t backwardMask = EnginePrecompute::backwardPawnMaskTable[c][square];
        uint64_t pawnThreatMask = PrecomputeMoves::pawnThreatTable[oc][nextSquare];

        // Is backward pawn
        if (board.isEmpty(nextSquare) && !(pawnsBitboard & backwardMask) && (board.getBitboard(Piece::PAWN, opposingColour) & pawnThreatMask)) {
            phasedEval += phase * BACKWARD_PAWN_PENALTY + (MAX_PHASE - phase) * BACKWARD_PAWN_PENALTY_END_GAME;
        }

        // Is part of a pawn chain
        uint64_t pawnChainMask = EnginePrecompute::pawnChainMaskTable[c][square];
        uint64_t pawnChainBitboard = pawnsBitboard & pawnChainMask;
        if (pawnChainBitboard) {
            uint8_t chainsCount = std::popcount(pawnChainBitboard); // Number of chains it is part of (1 or 2)
            phasedEval += chainsCount * (phase * PAWN_CHAIN_BONUS + (MAX_PHASE - phase) * PAWN_CHAIN_BONUS_END_GAME);
        }

        // Is a passed pawn
        uint64_t passedPawnMask = EnginePrecompute::passedPawnMaskTable[c][square];
        if (!(passedPawnMask & board.getBitboard(Piece::PAWN, opposingColour))) {
            uint8_t effectiveSquare = (colour == Colour::WHITE) ? square ^ 0x38 : square;
            phasedEval += (
                PieceTables::passedPawnTables[0][effectiveSquare] * phase + 
                PieceTables::passedPawnTables[1][effectiveSquare] * (MAX_PHASE - phase)
            );
        }

        pawnsBitboardTemp &= (pawnsBitboardTemp - 1);
    }

    // Major pawn shield bonus
    {
        Bitboard majorPawnsShieldBitboard = EnginePrecompute::majorPawnShieldTable[c][kingSquare] & pawnsBitboard;
        phasedEval += phase * std::popcount(majorPawnsShieldBitboard) * MAJOR_PAWN_SHIELD_BONUS;
    }

    // Minor pawn shield bonus
    {
        Bitboard minorPawnsShieldBitboard = EnginePrecompute::minorPawnShieldTable[c][kingSquare] & pawnsBitboard;
        phasedEval += phase * std::popcount(minorPawnsShieldBitboard) * MINOR_PAWN_SHIELD_BONUS;
    }

    // King tropism bonuses
    constexpr Piece tropismPieces[4] = {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
    for (uint8_t i = 0; i < 4; i++) {
        Bitboard bitboard = board.getBitboard(tropismPieces[i], colour);
        while (bitboard) {
            uint8_t square = std::countr_zero(bitboard);
            uint8_t distance = EnginePrecompute::chebyshevDistanceTable[opposingKingSquare][square];
            
            if (distance < MAX_TROPISM_DISTANCE) {
                phasedEval += phase * KING_TROPISM_BONUSES[i] * (MAX_TROPISM_DISTANCE - distance);
            }

            bitboard &= bitboard - 1;
        }
    }

    // Rook open/semi-open file bonus
    Bitboard rooksBitboardTemp = board.getBitboard(Piece::ROOK, colour);
    while (rooksBitboardTemp) {
        uint8_t square = std::countr_zero(rooksBitboardTemp);
        Bitboard openFileMask = EnginePrecompute::fileTable[square];
        
        // Open file
        if (!(allPawnsBitboard & openFileMask)) {
            phasedEval += phase * ROOK_OPEN_FILE_BONUS + (MAX_PHASE - phase) * ROOK_OPEN_FILE_BONUS_END_GAME;
        // Semi-open file
        } else if (!(pawnsBitboard & openFileMask)) {
            phasedEval += phase * ROOK_SEMI_OPEN_FILE_BONUS + (MAX_PHASE - phase) * ROOK_SEMI_OPEN_FILE_BONUS_END_GAME;
        }

        rooksBitboardTemp &= rooksBitboardTemp - 1;
    }

    // Queen open/semi-open file bonus
    Bitboard queensBitboardTemp = board.getBitboard(Piece::QUEEN, colour);
    while (queensBitboardTemp) {
        uint8_t square = std::countr_zero(queensBitboardTemp);
        Bitboard openFileMask = EnginePrecompute::fileTable[square];
        
        // Open file
        if (!(allPawnsBitboard & openFileMask)) {
            phasedEval += phase * QUEEN_OPEN_FILE_BONUS + (MAX_PHASE - phase) * QUEEN_OPEN_FILE_BONUS_END_GAME;
        // Semi-open file
        } else if (!(pawnsBitboard & openFileMask)) {
            phasedEval += phase * QUEEN_SEMI_OPEN_FILE_BONUS + (MAX_PHASE - phase) * QUEEN_SEMI_OPEN_FILE_BONUS_END_GAME;
        }

        queensBitboardTemp &= queensBitboardTemp - 1;
    }

    // Open file near king penalty
    {
        for (int offset = -1; offset <= 1; offset++) {
            int file = kingFile + offset;
            if (file < 0 || file > 7) continue;

            Bitboard openFileMask = EnginePrecompute::fileTable[kingSquare + offset];

            // Open file
            if (!(allPawnsBitboard & openFileMask)) {
                phasedEval += phase * OPEN_FILE_NEAR_KING_PENALTY;
            // Semi-open file
            } else if (!(pawnsBitboard & openFileMask)) {
                phasedEval += phase * SEMI_OPEN_FILE_NEAR_KING_PENALTY;
            }
        }
    }

    // Opposing pieces attacking in king zone penalty
    {
        constexpr Piece kingZoneAttacksPieces[5] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
        Bitboard kingZone = PrecomputeMoves::kingMoveTable[kingSquare];

        for (uint8_t i = 0; i < 4; i++) {
            Bitboard bitboard = board.getBitboard(kingZoneAttacksPieces[i], opposingColour);
            while (bitboard) {
                uint8_t square = std::countr_zero(bitboard);
                Bitboard attacks = 0ULL;
                switch (kingZoneAttacksPieces[i]) {
                    case Piece::PAWN:
                        attacks = PrecomputeMoves::pawnCaptureTable[oc][square];
                        break;
                    case Piece::KNIGHT:
                        attacks = PrecomputeMoves::knightMoveTable[square];
                        break;
                    case Piece::BISHOP:
                        attacks = PrecomputeMoves::getBishopMovesFromTable(square, allPiecesBitboard);
                        break;
                    case Piece::ROOK:
                        attacks = PrecomputeMoves::getRookMovesFromTable(square, allPiecesBitboard);
                        break;
                    case Piece::QUEEN:
                        attacks = PrecomputeMoves::getBishopMovesFromTable(square, allPiecesBitboard) |
                                    PrecomputeMoves::getRookMovesFromTable(square, allPiecesBitboard);
                        break;
                    default:
                        attacks = 0ULL;
                }

                Bitboard kingZoneAttacks = attacks & kingZone;
                phasedEval += phase * std::popcount(kingZoneAttacks) * KING_ZONE_ATTACK_PENALTIES[i];

                bitboard &= bitboard - 1;
            }
        }
    }

    // Bishop mobility bonus
    Bitboard bishopsBitboardTemp = board.getBitboard(Piece::BISHOP, colour);
    while (bishopsBitboardTemp) {
        uint8_t square = std::countr_zero(bishopsBitboardTemp);
        Bitboard bishopMoves = PrecomputeMoves::getBishopMovesFromTable(square, allPiecesBitboard);
        bishopMoves &= ~piecesBitboard; // Remove squares which land onto same colour pieces
        uint8_t mobility = std::popcount(bishopMoves); // Number of squares that the bishop can move to
        phasedEval += phase * BISHOP_MOBILITY_BONUSES[mobility] + (MAX_PHASE - phase) * BISHOP_MOBILITY_BONUSES_END_GAME[mobility];

        bishopsBitboardTemp &= bishopsBitboardTemp - 1;
    }

    // Knight mobility bonus
    Bitboard knightsBitboardTemp = board.getBitboard(Piece::KNIGHT, colour);
    while (knightsBitboardTemp) {
        uint8_t square = std::countr_zero(knightsBitboardTemp);
        Bitboard knightMoves = PrecomputeMoves::knightMoveTable[square];
        knightMoves &= ~piecesBitboard; // Remove squares which land onto same colour pieces
        uint8_t mobility = std::popcount(knightMoves); // Number of squares that the knight can move to
        phasedEval += phase * KNIGHT_MOBILITY_BONUSES[mobility] + (MAX_PHASE - phase) * KNIGHT_MOBILITY_BONUSES_END_GAME[mobility];

        knightsBitboardTemp &= knightsBitboardTemp - 1;
    }

    // Connected rooks bonus
    Bitboard rooksBitboardTemp2 = board.getBitboard(Piece::ROOK, colour);
    while (rooksBitboardTemp2) {
        uint8_t square1 = std::countr_zero(rooksBitboardTemp2);
        rooksBitboardTemp2 &= rooksBitboardTemp2 - 1;

        Bitboard remainingRooksBitboard = rooksBitboardTemp2;
        while (remainingRooksBitboard) {
            uint8_t square2 = std::countr_zero(remainingRooksBitboard);
            if (Board::getFile(square1) == Board::getFile(square2)) {
                uint64_t squaresBetweenMask = EnginePrecompute::sameFileSquaresBetweenTable[square1][square2];
                if (!(squaresBetweenMask & allPiecesBitboard)) {
                    phasedEval += phase * CONNECTED_ROOK_BONUS + (MAX_PHASE - phase) * CONNECTED_ROOK_BONUS_END_GAME;
                }
            } else if (Board::getRank(square1) == Board::getRank(square2)) {
                uint64_t squaresBetweenMask = EnginePrecompute::sameRankSquaresBetweenTable[square1][square2];
                if (!(squaresBetweenMask & allPiecesBitboard)) {
                    phasedEval += phase * CONNECTED_ROOK_BONUS + (MAX_PHASE - phase) * CONNECTED_ROOK_BONUS_END_GAME;
                }
            }

            remainingRooksBitboard &= remainingRooksBitboard - 1;
        }
    }

    for (uint8_t file = 0; file < 7; file++) {
        uint64_t currentFileMask = 0x0101010101010101ULL << file;
        uint64_t rightFileMask = 0x0101010101010101ULL << (file + 1);

        Bitboard currentFilePawnsBitboard = pawnsBitboard & currentFileMask;
        Bitboard rightFilePawnsBitboard = pawnsBitboard & rightFileMask;

        if (!currentFilePawnsBitboard || !rightFilePawnsBitboard) continue;

        uint8_t currentFilePawnSquare, rightFilePawnSquare;
        if (colour == Colour::WHITE) {
            currentFilePawnSquare = 63 - std::countl_zero(currentFilePawnsBitboard);
            rightFilePawnSquare = 63 - std::countl_zero(rightFilePawnsBitboard);
        } else {
            currentFilePawnSquare = std::countr_zero(currentFilePawnsBitboard);
            rightFilePawnSquare = std::countr_zero(rightFilePawnsBitboard);
        }

        uint8_t currentFilePawnRank = Board::getRank(currentFilePawnSquare);
        uint8_t rightFilePawnRank = Board::getRank(rightFilePawnSquare);

        uint8_t currentFilePawnAdvance = (colour == Colour::WHITE) ? currentFilePawnRank - 1 : 6 - currentFilePawnRank;
        uint8_t rightFilePawnAdvance = (colour == Colour::WHITE) ? rightFilePawnRank - 1 : 6 - rightFilePawnRank;

        // Pawns must be at least 3 squares forward from starting position to form a pawn storm
        if (currentFilePawnAdvance >= 3 && rightFilePawnAdvance >= 3) {
            // Pawns must be within 2 files of the king
            if (opposingKingFile >= file - 1 && opposingKingFile <= file + 2) {
                phasedEval += phase * PAWN_STORM_BONUS;

                uint8_t currentFilePawnRankDistance = (currentFilePawnRank >= opposingKingRank) ? 
                                                        currentFilePawnRank - opposingKingRank :
                                                        opposingKingRank - currentFilePawnRank;

                uint8_t rightFilePawnRankDistance = (rightFilePawnRank >= opposingKingRank) ? 
                                                        rightFilePawnRank - opposingKingRank :
                                                        opposingKingRank - rightFilePawnRank;

                uint8_t distance = (currentFilePawnRankDistance <= rightFilePawnRankDistance) ?
                                    currentFilePawnRankDistance :
                                    rightFilePawnRankDistance;

                // Bonus for pawn storms close to the king
                if (distance <= 2) {
                    phasedEval += phase * PAWN_STORM_PROXIMITY_BONUS * (3 - distance);
                }

                break; // Only include 1 pawn storm in evaluation
            }
        }
    }

    eval += phasedEval / MAX_PHASE;

    return eval;
}

void Evaluation::orderMoves(std::vector<Move>& moves, Board& board, uint8_t ply, Colour colour, const Move* bestMove) {
    static std::vector<std::pair<Move, std::pair<MoveType, int32_t>>> scoredMovesBuffer;
    scoredMovesBuffer.clear();
    scoredMovesBuffer.reserve(moves.size());

    for (Move move : moves) {
        auto moveScore = orderingScore(move, board, ply, colour, bestMove);
        scoredMovesBuffer.push_back({move, moveScore});
    }

    std::sort(scoredMovesBuffer.begin(), scoredMovesBuffer.end(), [](const auto& a, const auto& b) {
        // Moves are of different types
        if (a.second.first != b.second.first) {
            return a.second.first < b.second.first;
        }
        return a.second.second > b.second.second;
    });

    for (int i = 0; i < moves.size(); i++) {
        moves[i] = scoredMovesBuffer[i].first;
    }
}

void Evaluation::orderQuiescenceMoves(std::vector<Move>& moves, Board& board) {
    std::sort(moves.begin(), moves.end(), [&board](const Move a, const Move b) {
        return orderingQuiescenceScore(a, board) > orderingQuiescenceScore(b, board);
    });
}

int16_t Evaluation::evaluate(Game& game, GameStateEvaluation state, uint8_t ply) {
    if (state == GameStateEvaluation::CHECKMATE) return -CHECKMATE_VALUE + ply;
    
    // Stalemate / Draw by either fifty move rule, repetition or insufficient material
    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        return 0;
    }

    Board& board = game.getBoard();
    Colour currentColour = game.getCurrentTurn();
    Colour opposingColour = (currentColour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    int16_t phase = gamePhase(board);
    int16_t eval = pieceValueEvaluation(board, currentColour, phase) - pieceValueEvaluation(board, opposingColour, phase);

    return eval;
}

void Evaluation::addKillerMove(Move move, uint8_t ply) {
    if (killerMoves[ply][0] != move && killerMoves[ply][1] != move) {
        killerMoves[ply][1] = killerMoves[ply][0];
        killerMoves[ply][0] = move;
    }
}

void Evaluation::clearKillerMoveTable() {
    std::memset(killerMoves, 0, sizeof(killerMoves));
}

bool Evaluation::isKillerMove(Move move, uint8_t ply) {
    return (killerMoves[ply][0] == move || killerMoves[ply][1] == move);
}

void Evaluation::addHistoryHeuristic(Move move, Piece piece, Colour colour, uint8_t depth) {
    uint8_t c = toIndex(colour);
    uint8_t p = toIndex(piece);
    int16_t& entry = historyHeuristics[c][p][move.getFromSquare()][move.getToSquare()];
    entry += depth * depth;

    if (entry > MAX_HISTORY_VALUE) entry = MAX_HISTORY_VALUE;
}

void Evaluation::ageHistoryHeuristicsTable() {
    for (uint8_t colour = 0; colour < 2; colour++) {
        for (uint8_t piece = 0; piece < 6; piece++) {
            for (uint8_t from = 0; from < 64; from++) {
                for (uint8_t to = 0; to < 64; to++) {
                    int16_t& entry = historyHeuristics[colour][piece][from][to];
                    entry = 3 * entry / 4;
                }
            }
        }
    }
}

void Evaluation::clearHistoryHeuristicsTable() {
    std::memset(historyHeuristics, 0, sizeof(historyHeuristics));
}

std::pair<MoveType, int16_t> Evaluation::orderingScore(const Move move, Board& board, uint8_t ply, Colour colour, const Move* bestMove) {
    if (bestMove && move == *bestMove) return {MoveType::BEST, 0};
    if (move == killerMoves[ply][0]) return {MoveType::KILLER, 1};
    if (move == killerMoves[ply][1]) return {MoveType::KILLER, 0};

    // Queen promotion moves
    uint8_t promotionPiece = move.getPromotionPiece();
    if (promotionPiece == toIndex(Piece::QUEEN)) {
        int16_t promotionScore = 0;
        uint8_t capturedPiece = move.getCapturedPiece();

        // Bonus for promotion capture
        if (capturedPiece != Move::NO_CAPTURE) {
            Piece attacker = board.getPiece(move.getFromSquare());
            promotionScore += 10 * pieceEvals[capturedPiece] - pieceEvals[toIndex(attacker)];
        }

        return {MoveType::PROMOTION, promotionScore};
    }

    // Non queen promotion capture moves
    uint8_t capturedPiece = move.getCapturedPiece();
    if (capturedPiece != Move::NO_CAPTURE) {
        int16_t captureScore = 0;
        Piece attacker = board.getPiece(move.getFromSquare());
        captureScore += 10 * pieceEvals[capturedPiece] - pieceEvals[toIndex(attacker)];
        return {MoveType::CAPTURE, captureScore};
    }

    // History heuristic moves
    uint8_t f = move.getFromSquare();
    uint8_t t = move.getToSquare();
    uint8_t c = toIndex(colour);
    uint8_t p = toIndex(board.getPiece(f));
    int16_t historyScore = historyHeuristics[c][p][f][t];
    return {MoveType::HISTORY, historyScore};
}

int16_t Evaluation::orderingQuiescenceScore(const Move move, Board& board) {
    int16_t score = 0;
    uint8_t capturedPiece = move.getCapturedPiece();
    if (capturedPiece != Move::NO_CAPTURE) {
        Piece attacker = board.getPiece(move.getFromSquare());
        score += 10 * pieceEvals[capturedPiece] - pieceEvals[toIndex(attacker)];
    }

    uint8_t promotionPiece = move.getPromotionPiece();
    if (promotionPiece == toIndex(Piece::QUEEN)) {
        score += PROMOTION_ORDERING_VALUE;
    }

    return score;
}