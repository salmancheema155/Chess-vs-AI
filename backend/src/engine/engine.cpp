#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <bit>
#include "engine/engine.h"
#include "board/board.h"
#include "move/move.h"
#include "game/game.h"
#include "move/move_generator.h"
#include "chess_types.h"

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::Bitboard;
using Chess::toIndex;

namespace {
    constexpr int CHECKMATE_VALUE = 32000;
    constexpr int PAWN_VALUE = 100;
    constexpr int KNIGHT_VALUE = 320;
    constexpr int BISHOP_VALUE = 330;
    constexpr int ROOK_VALUE = 500;
    constexpr int QUEEN_VALUE = 900;
    constexpr int pieceEvals[5] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE};
}

namespace {
    int pieceValueEvaluation(Board& board, Colour colour) {
        constexpr Piece pieces[5] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
        int eval = 0;

        for (Piece piece : pieces) {
            Bitboard bitboard = board.getBitboard(piece, colour);
            while (bitboard != 0) {
                eval += pieceEvals[toIndex(piece)];
                bitboard &= (bitboard - 1);
            }
        }

        return eval;
    }

    int orderingScore(const Move& move, Board& board) {
        int score = 0;

        uint8_t capturedPiece = move.getCapturedPiece();
        if (capturedPiece != Move::NO_CAPTURE) {
            Piece attacker = board.getPiece(move.getFromSquare());
            score += 10 * pieceEvals[capturedPiece] - pieceEvals[toIndex(attacker)];
        }

        uint8_t promotionPiece = move.getPromotionPiece();
        if (promotionPiece != Move::NO_PROMOTION) {
            score += pieceEvals[promotionPiece] + 800;
        }

        return score;
    }

    void orderMoves(std::vector<Move>& moves, Board& board) {
        std::sort(moves.begin(), moves.end(), [&board](const Move& a, const Move& b) {
            return orderingScore(a, board) > orderingScore(b, board); 
        });
    }
}

Move Engine::getMove(Game& game, int depth) {
    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    Move bestMove;
    int bestEval = (colour == Colour::WHITE) ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    std::vector<Move> moves = MoveGenerator::legalMoves(board, colour);
    orderMoves(moves, board);
    
    for (const Move& move : moves) {
        game.makeMove(move);
        int eval = minimax(game, depth - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        game.undo();

        if (colour == Colour::WHITE) {
            if (eval >= bestEval) {
                bestEval = eval;
                bestMove = move;
            }
        } else {
            if (eval <= bestEval) {
                bestEval = eval;
                bestMove = move;
            }
        }
    }

    return bestMove;
}

int Engine::minimax(Game& game, int depth, int alpha, int beta) {
    GameStateEvaluation state = game.getCurrentGameStateEvaluation();
    if (depth == 0 || (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK)) {
        return evaluate(game, state, depth);
    }

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    std::vector<Move> moves = MoveGenerator::legalMoves(board, colour);
    orderMoves(moves, board);

    if (colour == Colour::WHITE) {
        int maxEval = std::numeric_limits<int>::min();
        for (const Move& move : moves) {
            game.makeMove(move);
            int eval = minimax(game, depth - 1, alpha, beta);
            game.undo();

            if (eval > maxEval) maxEval = eval;
            if (eval > alpha) alpha = eval;
            if (beta <= alpha) break;
        }
        
        return maxEval;

    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const Move& move : moves) {
            game.makeMove(move);
            int eval = minimax(game, depth - 1, alpha, beta);
            game.undo();

            if (eval < minEval) minEval = eval;
            if (eval < beta) beta = eval;
            if (beta <= alpha) break;
        }

        return minEval;
    }
}

int Engine::evaluate(Game& game, GameStateEvaluation& state, int depth) {
    if (state == GameStateEvaluation::CHECKMATE) {
        // Current turn = white means turn just switched to white after last move
        return (game.getCurrentTurn() == Colour::WHITE) ? -CHECKMATE_VALUE - depth: CHECKMATE_VALUE + depth;
    }
    
    // Stalemate / Draw by either fifty move rule, repetition or insufficient material
    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        return 0;
    }

    Board& board = game.getBoard();
    int eval = pieceValueEvaluation(board, Colour::WHITE) - pieceValueEvaluation(board, Colour::BLACK);

    return eval;
}