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

namespace {
    double gamePhase(Board& board) {
        constexpr int MAX_PHASE = 24;
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
        
        if (totalPhase >= MAX_PHASE) return 1.0;
        return ((double)totalPhase) / MAX_PHASE;
    }
}

int16_t Evaluation::pieceValueEvaluation(Board& board, Colour colour, double phase) {
    constexpr Piece pieces[6] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN, Piece::KING};
    const Colour opposingColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    const uint8_t kingSquare = board.getKingSquare(colour);
    const uint8_t opposingKingSquare = board.getKingSquare(opposingColour);

    const Bitboard allPiecesBitboard = board.getPiecesBitboard();
    const Bitboard bitboard = board.getBitboard(colour);
    const Bitboard pawnsBitboard = board.getBitboard(Piece::PAWN, colour);
    const Bitboard opposingPawnsBitboard = board.getBitboard(Piece::PAWN, opposingColour);
    const Bitboard allPawnsBitboard = pawnsBitboard | opposingPawnsBitboard;

    uint8_t c = toIndex(colour);
    int16_t eval = 0;

    for (uint8_t i = 0; i < 6; i++) {
        Bitboard bitboard = board.getBitboard(pieces[i], colour);
        while (bitboard) {
            // Piece evaluation
            eval += pieceEvals[i];

            // Piece Square Table evaluation
            uint8_t square = std::countr_zero(bitboard);
            if (colour == Colour::WHITE) square ^= 0x38; // Flip square from black to white's perspective
            eval += static_cast<int16_t>(PieceTables::tables[i][square] * phase + PieceTables::endgameTables[i][square] * (1 - phase));

            bitboard &= (bitboard - 1);
        }
    }

    // Pawn structure penalties
    for (uint8_t file = 0; file < 8; file++) {
        uint64_t mask = 0x0101010101010101ULL << file; // Mask of the current file
        uint8_t pawnCount = std::popcount(pawnsBitboard & mask);

        // Penalty for doubling pawns
        double doublingPenalty = (pawnCount - 1) * (phase * DOUBLED_PAWN_PENALTY + (1 - phase) * DOUBLED_PAWN_PENALTY_END_GAME);
        eval += static_cast<int16_t>(doublingPenalty);

        // Penalty for isolated pawns
        uint64_t isolatedMask = EnginePrecompute::adjacentFileMaskTable[file];
        if (pawnsBitboard & isolatedMask) continue; // Contains pawn on either immediate left or right file
        double isolatedPenalty =  pawnCount * (phase * ISOLATED_PAWN_PENALTY + (1 - phase) * ISOLATED_PAWN_PENALTY_END_GAME);
        eval += static_cast<int16_t>(isolatedPenalty);
    }

    Bitboard pawnsBitboardTemp = pawnsBitboard;
    while (pawnsBitboardTemp) {
        uint8_t square = std::countr_zero(pawnsBitboardTemp);
        uint8_t nextSquare = (colour == Colour::WHITE) ? square + 8 : square - 8;
        uint64_t backwardMask = EnginePrecompute::backwardPawnMaskTable[c][square];
        uint64_t pawnThreatMask = PrecomputeMoves::pawnThreatTable[toIndex(opposingColour)][nextSquare];

        // Is backward pawn
        if (board.isEmpty(nextSquare) && !(pawnsBitboard & backwardMask) && (board.getBitboard(Piece::PAWN, opposingColour) & pawnThreatMask)) {
            double backwardPenalty = (phase * BACKWARD_PAWN_PENALTY + (1 - phase) * BACKWARD_PAWN_PENALTY_END_GAME);
            eval += static_cast<int16_t>(backwardPenalty);
        }

        // Is part of a pawn chain
        uint64_t pawnChainMask = EnginePrecompute::pawnChainMaskTable[c][square];
        uint64_t pawnChainBitboard = pawnsBitboard & pawnChainMask;
        if (pawnChainBitboard) {
            uint8_t chainsCount = std::popcount(pawnChainBitboard); // Number of chains it is part of (1/2)
            double pawnChainBonus = chainsCount * (phase * PAWN_CHAIN_BONUS + (1 - phase) * PAWN_CHAIN_BONUS_END_GAME);
            eval += static_cast<int16_t>(pawnChainBonus);
        }

        // Is a passed pawn
        uint64_t passedPawnMask = EnginePrecompute::passedPawnMaskTable[c][square];
        if (!(passedPawnMask & board.getBitboard(Piece::PAWN, opposingColour))) {
            uint8_t effectiveSquare = (colour == Colour::WHITE) ? square ^ 0x38 : square;
            eval += static_cast<int16_t>(
                PieceTables::passedPawnTables[0][effectiveSquare] * phase + 
                PieceTables::passedPawnTables[1][effectiveSquare] * (1 - phase)
            );
        }

        pawnsBitboardTemp &= (pawnsBitboardTemp - 1);
    }

    // Major pawn shield bonus
    Bitboard majorPawnsShieldBitboard = EnginePrecompute::majorPawnShieldTable[c][kingSquare] & pawnsBitboard;
    double majorPawnShieldBonus = phase * std::popcount(majorPawnsShieldBitboard) * MAJOR_PAWN_SHIELD_BONUS;
    eval += static_cast<int16_t>(majorPawnShieldBonus);

    // Minor pawn shield bonus
    Bitboard minorPawnsShieldBitboard = EnginePrecompute::minorPawnShieldTable[c][kingSquare] & pawnsBitboard;
    double minorPawnShieldBonus = phase * std::popcount(minorPawnsShieldBitboard) * MINOR_PAWN_SHIELD_BONUS;
    eval += static_cast<int16_t>(minorPawnShieldBonus);

    // King tropism penalties
    constexpr Piece tropismPieces[4] = {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
    double tropismBonus = 0.0;
    for (uint8_t i = 0; i < 4; i++) {
        Bitboard bitboard = board.getBitboard(tropismPieces[i], colour);
        while (bitboard) {
            uint8_t square = std::countr_zero(bitboard);
            uint8_t distance = EnginePrecompute::chebyshevDistanceTable[opposingKingSquare][square];
            
            if (distance < MAX_TROPISM_DISTANCE) {
                tropismBonus += phase * KING_TROPISM_BONUSES[i] * (MAX_TROPISM_DISTANCE - distance);
            }

            bitboard &= bitboard - 1;
        }
    }
    eval += static_cast<int16_t>(tropismBonus);

    // Rook open/semi-open file bonus
    Bitboard rooksBitboardTemp = board.getBitboard(Piece::ROOK, colour);
    double rookOpenFileBonus = 0.0;
    while (rooksBitboardTemp) {
        uint8_t square = std::countr_zero(rooksBitboardTemp);
        Bitboard openFileMask = EnginePrecompute::openFileTable[square];
        
        // Open file
        if (!(allPawnsBitboard & openFileMask)) {
            rookOpenFileBonus += phase * ROOK_OPEN_FILE_BONUS + (1 - phase) * ROOK_OPEN_FILE_BONUS_END_GAME;
        // Semi-open file
        } else if (!(opposingPawnsBitboard & openFileMask)) {
            rookOpenFileBonus += phase * ROOK_SEMI_OPEN_FILE_BONUS + (1 - phase) * ROOK_SEMI_OPEN_FILE_BONUS_END_GAME;
        }

        rooksBitboardTemp &= rooksBitboardTemp - 1;
    }
    eval += static_cast<int16_t>(rookOpenFileBonus);

    // Queen open/semi-open file bonus
    Bitboard queensBitboardTemp = board.getBitboard(Piece::QUEEN, colour);
    double queenOpenFileBonus = 0.0;
    while (queensBitboardTemp) {
        uint8_t square = std::countr_zero(queensBitboardTemp);
        Bitboard openFileMask = EnginePrecompute::openFileTable[square];
        
        // Open file
        if (!(allPawnsBitboard & openFileMask)) {
            queenOpenFileBonus += phase * QUEEN_OPEN_FILE_BONUS + (1 - phase) * QUEEN_OPEN_FILE_BONUS_END_GAME;
        // Semi-open file
        } else if (!(opposingPawnsBitboard & openFileMask)) {
            queenOpenFileBonus += phase * QUEEN_SEMI_OPEN_FILE_BONUS + (1 - phase) * QUEEN_SEMI_OPEN_FILE_BONUS_END_GAME;
        }

        queensBitboardTemp &= queensBitboardTemp - 1;
    }
    eval += static_cast<int16_t>(queenOpenFileBonus);

    // Bishop mobility bonus
    Bitboard bishopsBitboardTemp = board.getBitboard(Piece::BISHOP, colour);
    double bishopMobilityBonus = 0.0;
    while (bishopsBitboardTemp) {
        uint8_t square = std::countr_zero(bishopsBitboardTemp);
        Bitboard bishopMoves = PrecomputeMoves::getBishopMovesFromTable(square, allPiecesBitboard);
        bishopMoves &= ~bitboard; // Remove squares which land onto same colour pieces
        uint8_t mobility = std::popcount(bishopMoves); // Number of squares that the bishop can move to
        bishopMobilityBonus += phase * BISHOP_MOBILITY_BONUSES[mobility] + (1 - phase) * BISHOP_MOBILITY_BONUSES_END_GAME[mobility];

        bishopsBitboardTemp &= bishopsBitboardTemp - 1;
    }
    eval += static_cast<int16_t>(bishopMobilityBonus);

    // Knight mobility bonus
    Bitboard knightsBitboardTemp = board.getBitboard(Piece::KNIGHT, colour);
    double knightMobilityBonus = 0.0;
    while (knightsBitboardTemp) {
        uint8_t square = std::countr_zero(knightsBitboardTemp);
        Bitboard knightMoves = PrecomputeMoves::knightMoveTable[square];
        knightMoves &= ~bitboard; // Remove squares which land onto same colour pieces
        uint8_t mobility = std::popcount(knightMoves); // Number of squares that the knight can move to
        knightMobilityBonus += phase * KNIGHT_MOBILITY_BONUSES[mobility] + (1 - phase) * KNIGHT_MOBILITY_BONUSES_END_GAME[mobility];

        knightsBitboardTemp &= knightsBitboardTemp - 1;
    }
    eval += static_cast<int16_t>(knightMobilityBonus);

    // Connected rooks bonus
    Bitboard rooksBitboardTemp2 = board.getBitboard(Piece::ROOK, colour);
    double connectedRookBonus = 0.0;
    while (rooksBitboardTemp2) {
        uint8_t square1 = std::countr_zero(rooksBitboardTemp2);
        rooksBitboardTemp2 &= rooksBitboardTemp2 - 1;

        Bitboard remainingRooksBitboard = rooksBitboardTemp2;
        while (remainingRooksBitboard) {
            uint8_t square2 = std::countr_zero(remainingRooksBitboard);
            if (Board::getFile(square1) == Board::getFile(square2)) {
                uint64_t squaresBetweenMask = EnginePrecompute::sameFileSquaresBetweenTable[square1][square2];
                if (!(squaresBetweenMask & allPiecesBitboard)) {
                    connectedRookBonus += phase * CONNECTED_ROOK_BONUS + (1 - phase) * CONNECTED_ROOK_BONUS_END_GAME;
                }
            } else if (Board::getRank(square1) == Board::getRank(square2)) {
                uint64_t squaresBetweenMask = EnginePrecompute::sameRankSquaresBetweenTable[square1][square2];
                if (!(squaresBetweenMask & allPiecesBitboard)) {
                    connectedRookBonus += phase * CONNECTED_ROOK_BONUS + (1 - phase) * CONNECTED_ROOK_BONUS_END_GAME;
                }
            }

            remainingRooksBitboard &= remainingRooksBitboard - 1;
        }
    }
    eval += static_cast<int16_t>(connectedRookBonus);

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
    double phase = gamePhase(board);
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