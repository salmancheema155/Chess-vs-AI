#include <vector>
#include <cstdint>
#include <limits>
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
    constexpr int PAWN_VALUE = 100;
    constexpr int KNIGHT_VALUE = 320;
    constexpr int BISHOP_VALUE = 330;
    constexpr int ROOK_VALUE = 500;
    constexpr int QUEEN_VALUE = 900; 
}

namespace {
    int pieceValueEvaluation(Board& board, Colour colour) {
        constexpr Piece pieces[5] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
        constexpr int pieceEvals[5] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE};
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
}

Move Engine::getMove(Game& game, int depth) {
    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    Move bestMove;
    int bestEval = (colour == Colour::WHITE) ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    constexpr Piece pieces[6] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN, Piece::KING};

    for (const Piece piece : pieces) {
        Bitboard bitboard = board.getBitboard(piece, colour);
        while (bitboard != 0) {
            uint8_t square = std::countr_zero(bitboard);
            std::vector<Move> moves = MoveGenerator::legalMoves(board, piece, colour, square);
            
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

            bitboard &= (bitboard - 1);
        }
    }

    return bestMove;
}

int Engine::minimax(Game& game, int depth, int alpha, int beta) {
    GameStateEvaluation state = game.getCurrentGameStateEvaluation();
    if (depth == 0 || (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK)) {
        return evaluate(game, state);
    }

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    constexpr Piece pieces[6] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN, Piece::KING};

    if (colour == Colour::WHITE) {
        int maxEval = std::numeric_limits<int>::min();
        for (const Piece piece : pieces) {
            Bitboard bitboard = board.getBitboard(piece, colour);
            while (bitboard != 0) {
                uint8_t square = std::countr_zero(bitboard);
                std::vector<Move> moves = MoveGenerator::legalMoves(board, piece, colour, square);

                for (const Move& move : moves) {
                    game.makeMove(move);
                    int eval = minimax(game, depth - 1, alpha, beta);
                    game.undo();

                    if (eval > maxEval) maxEval = eval;
                    if (eval > alpha) alpha = eval;
                    if (beta <= alpha) goto pruning_max_finish;
                }

                bitboard &= (bitboard - 1);
            }
        }

        pruning_max_finish:
            return maxEval;

    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const Piece piece : pieces) {
            Bitboard bitboard = board.getBitboard(piece, colour);
            while (bitboard != 0) {
                uint8_t square = std::countr_zero(bitboard);
                std::vector<Move> moves = MoveGenerator::legalMoves(board, piece, colour, square);

                for (const Move& move : moves) {
                    game.makeMove(move);
                    int eval = minimax(game, depth - 1, alpha, beta);
                    game.undo();

                    if (eval < minEval) minEval = eval;
                    if (eval < beta) beta = eval;
                    if (beta <= alpha) goto pruning_min_finish;
                }

                bitboard &= (bitboard - 1);
            }
        }

        pruning_min_finish:
            return minEval;
    }
}

int Engine::evaluate(Game& game, GameStateEvaluation& state) {
    if (state == GameStateEvaluation::CHECKMATE) {
        // Current turn = white means turn just switched to white after last move
        return (game.getCurrentTurn() == Colour::WHITE) ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    }
    
    // Stalemate / Draw by either fifty move rule, repetition or insufficient material
    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        return 0;
    }

    Board& board = game.getBoard();
    Bitboard whiteBitboard = board.getBitboard(Colour::WHITE);
    Bitboard blackBitboard = board.getBitboard(Colour::BLACK);
    int eval = pieceValueEvaluation(board, Colour::WHITE) - pieceValueEvaluation(board, Colour::BLACK);

    return eval;
}