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
    int bestEval = std::numeric_limits<int>::min();

    std::vector<Move> moves = MoveGenerator::legalMoves(board, colour);
    orderMoves(moves, board);
    
    for (const Move& move : moves) {
        game.makeMove(move);
        int eval = -negamax(game, depth - 1, std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max());
        game.undo();

        if (eval > bestEval) {
            bestEval = eval;
            bestMove = move;
        }
    }

    return bestMove;
}

int Engine::negamax(Game& game, int depth, int alpha, int beta) {
    GameStateEvaluation state = game.getCurrentGameStateEvaluation();
    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        return evaluate(game, state, depth);
    }

    if (depth == 0) return quiescence(game, alpha, beta, 8, state);

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    std::vector<Move> moves = MoveGenerator::legalMoves(board, colour);
    orderMoves(moves, board);

    int maxEval = std::numeric_limits<int>::min();
    for (const Move& move : moves) {
        game.makeMove(move);
        int eval = -negamax(game, depth - 1, -beta, -alpha);
        game.undo();

        if (eval > maxEval) maxEval = eval;
        if (eval > alpha) alpha = eval;
        if (beta <= alpha) break;
    }
    
    return maxEval;
}

int Engine::evaluate(Game& game, GameStateEvaluation state, int depth) {
    if (state == GameStateEvaluation::CHECKMATE) return -CHECKMATE_VALUE - depth;
    
    // Stalemate / Draw by either fifty move rule, repetition or insufficient material
    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        return 0;
    }

    Board& board = game.getBoard();
    Colour currentColour = game.getCurrentTurn();
    Colour opposingColour = (currentColour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    int eval = pieceValueEvaluation(board, currentColour) - pieceValueEvaluation(board, opposingColour);

    return eval;
}

int Engine::quiescence(Game& game, int alpha, int beta, int qdepth, GameStateEvaluation state) {
    if (qdepth == 0 || (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK)) {
        return evaluate(game, state, qdepth);
    }

    int currentEval = evaluate(game, state, 0);
    if (currentEval >= beta) return beta;
    if (currentEval > alpha) alpha = currentEval;

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    std::vector<Move> captureMoves = MoveGenerator::legalCaptures(board, colour);
    orderMoves(captureMoves, board);

    for (const Move& move : captureMoves) {
        game.makeMove(move);
        state = game.getCurrentGameStateEvaluation();
        int eval = -quiescence(game, -beta, -alpha, qdepth - 1, state);
        game.undo();

        if (eval >= beta) return beta;
        if (eval > alpha) alpha = eval;
    }

    return alpha;
}