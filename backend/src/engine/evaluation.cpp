#include <cstdint>
#include <bit>
#include <algorithm>
#include "board/board.h"
#include "move/move.h"
#include "game/game.h"
#include "chess_types.h"
#include "engine/evaluation.h"
#include "engine/piece_tables.h"
#include "engine/precompute.h"

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::Bitboard;

namespace {
    double gamePhase(Board& board) {
        constexpr int MAX_PHASE = 16;
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
    int16_t eval = 0;

    for (uint8_t i = 0; i < 6; i++) {
        Bitboard bitboard = board.getBitboard(pieces[i], colour);
        while (bitboard != 0) {
            // Piece evaluation
            eval += pieceEvals[i];

            // Piece Square Table evaluation
            uint8_t square = std::countr_zero(bitboard);
            if (colour == Colour::WHITE) square ^= 0x38; // Flip square from black to white's perspective
            eval += static_cast<int>(PieceTables::tables[i][square] * phase + PieceTables::endgameTables[i][square] * (1 - phase));

            bitboard &= (bitboard - 1);
        }
    }

    // Pawn structure penalties
    Bitboard pawnsBitboard = board.getBitboard(Piece::PAWN, colour);
    for (uint8_t file = 0; file < 8; file++) {
        uint64_t mask = 0x0101010101010101ULL << file;
        uint8_t pawnCount = std::popcount(pawnsBitboard & mask);

        // Penalty for doubling pawns
        double doublingPenalty = (pawnCount - 1) * (phase * DOUBLED_PAWN_PENALTY + (1 - phase) * DOUBLED_PAWN_PENALTY_END_GAME);
        eval += static_cast<int>(doublingPenalty);

        // Penalty for isolated pawns
        uint64_t isolatedMask = EnginePrecompute::isolatedPawnMaskTable[file];
        if (pawnsBitboard & isolatedMask) continue; // Contains pawn on either immediate left or right file
        double isolatedPenalty =  pawnCount * (phase * ISOLATED_PAWN_PENALTY + (1 - phase) * ISOLATED_PAWN_PENALTY_END_GAME);
        eval += static_cast<int>(isolatedPenalty);
    }

    Bitboard pawnsBitboardTemp = pawnsBitboard;
    while (pawnsBitboardTemp) {
        uint8_t square = std::countr_zero(pawnsBitboardTemp);
        uint64_t backwardMask = EnginePrecompute::backwardPawnMaskTable[toIndex(colour)][square];

        // Does not contain pawns on adjacent files either next to or behind ranks
        if (!(pawnsBitboard & backwardMask)) {
            double backwardPenalty = (phase * BACKWARD_PAWN_PENALTY + (1 - phase) * BACKWARD_PAWN_PENALTY_END_GAME);
            eval += static_cast<int>(backwardPenalty);
        }

        uint64_t connectedMask = EnginePrecompute::connectedPawnMaskTable[toIndex(colour)][square];

        // Has a connected pawn
        uint64_t connectedBitboard = pawnsBitboard & connectedMask;
        if (connectedBitboard) {
            int connectedCount = std::popcount(connectedBitboard);
            double connectedBonus = connectedCount * (phase * CONNECTED_PAWN_BONUS + (1 - phase) * CONNECTED_PAWN_BONUS_END_GAME);
            eval += static_cast<int>(connectedBonus);
        }

        pawnsBitboardTemp &= (pawnsBitboardTemp - 1);
    }

    return eval;
}

void Evaluation::orderMoves(std::vector<Move>& moves, Board& board, const Move* bestMove) {
    std::sort(moves.begin(), moves.end(), [&board, bestMove](const Move a, const Move b) {
        return orderingScore(a, board, bestMove) > orderingScore(b, board, bestMove); 
    });
}

void Evaluation::orderQuiescenceMoves(std::vector<Move>& moves, Board& board) {
    std::sort(moves.begin(), moves.end(), [&board](const Move a, const Move b) {
        return orderingScore(a, board) > orderingScore(b, board); 
    });
}

int16_t Evaluation::evaluate(Game& game, GameStateEvaluation state) {
    if (state == GameStateEvaluation::CHECKMATE) return -CHECKMATE_VALUE;
    
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

int16_t Evaluation::orderingScore(const Move move, Board& board, const Move* bestMove) {
    int16_t score = 0;
    uint8_t capturedPiece = move.getCapturedPiece();
    if (capturedPiece != Move::NO_CAPTURE) {
        Piece attacker = board.getPiece(move.getFromSquare());
        score += 10 * pieceEvals[capturedPiece] - pieceEvals[toIndex(attacker)];
    }

    uint8_t promotionPiece = move.getPromotionPiece();
    if (promotionPiece != Move::NO_PROMOTION) {
        score += pieceEvals[promotionPiece] + 800;
    }

    if (bestMove && move == *bestMove) {
        score += BEST_MOVE_VALUE;
    }

    return score;
}