#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <bit>
#include "engine/engine.h"
#include "engine/transposition_table.h"
#include "board/board.h"
#include "move/move.h"
#include "game/game.h"
#include "move/move_generator.h"
#include "chess_types.h"

// #include <iostream>

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::Bitboard;
using Chess::toIndex;

TranspositionTable Engine::transpositionTable(1024);
TranspositionTable Engine::quiescenceTranspositionTable(32);

// uint64_t probes = 0, hits = 0;

namespace {
    constexpr int16_t CHECKMATE_VALUE = 32000;
    constexpr int16_t PAWN_VALUE = 100;
    constexpr int16_t KNIGHT_VALUE = 320;
    constexpr int16_t BISHOP_VALUE = 330;
    constexpr int16_t ROOK_VALUE = 500;
    constexpr int16_t QUEEN_VALUE = 900;
    constexpr int16_t pieceEvals[5] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE};

    constexpr int16_t BEST_MOVE_VALUE = 30000;
}

namespace {
    int16_t pieceValueEvaluation(Board& board, Colour colour) {
        constexpr Piece pieces[5] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
        int16_t eval = 0;

        for (Piece piece : pieces) {
            Bitboard bitboard = board.getBitboard(piece, colour);
            while (bitboard != 0) {
                eval += pieceEvals[toIndex(piece)];
                bitboard &= (bitboard - 1);
            }
        }

        return eval;
    }

    int16_t orderingScore(const Move& move, Board& board) {
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

        return score;
    }

    void orderMoves(std::vector<Move>& moves, Board& board) {
        std::sort(moves.begin(), moves.end(), [&board](const Move& a, const Move& b) {
            return orderingScore(a, board) > orderingScore(b, board); 
        });
    }

    void orderQuiescenceMoves(std::vector<Move>& moves, Board& board) {
        std::sort(moves.begin(), moves.end(), [&board](const Move& a, const Move& b) {
            return orderingScore(a, board) > orderingScore(b, board); 
        });
    }
}

Move Engine::getMove(Game& game, uint8_t depth) {
    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    Move bestMove;
    int16_t bestEval = std::numeric_limits<int16_t>::min();

    std::vector<Move> moves = MoveGenerator::legalMoves(board, colour);
    orderMoves(moves, board);
    
    for (const Move& move : moves) {
        game.makeMove(move);
        int16_t eval = -negamax(game, depth - 1, std::numeric_limits<int16_t>::min() + 1, std::numeric_limits<int16_t>::max());
        game.undo();

        if (eval > bestEval) {
            bestEval = eval;
            bestMove = move;
        }
    }

    transpositionTable.incrementGeneration();
    quiescenceTranspositionTable.incrementGeneration();

    // std::cout << "Probes: " << probes << std::endl;
    // std::cout << "Hits: " << hits << std::endl;
    // std::cout << "Hit rate: " << 100 * (double)hits / (double)probes << "%" << std::endl;

    return bestMove;
}

int16_t Engine::negamax(Game& game, uint8_t depth, int16_t alpha, int16_t beta) {
    uint64_t hash = game.getHash();
    TTEntry* entry = transpositionTable.getEntry(hash);
    // probes++;
    if (entry && entry->depth >= depth) {
        // hits++;
        if (entry->flag == TTFlag::EXACT) return entry->eval;
        if (entry->flag == TTFlag::LOWER_BOUND && entry->eval >= beta) return entry->eval;
        if (entry->flag == TTFlag::UPPER_BOUND && entry->eval <= alpha) return entry->eval;
    }

    GameStateEvaluation state = game.getCurrentGameStateEvaluation();
    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        return evaluate(game, state, depth);
    }

    if (depth == 0) return quiescence(game, alpha, beta, 4, state);

    int16_t originalAlpha = alpha;
    int16_t maxEval = std::numeric_limits<int16_t>::min();
    Move bestMove;

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    std::vector<Move> moves = MoveGenerator::legalMoves(board, colour);
    orderMoves(moves, board);

    for (const Move& move : moves) {
        game.makeMove(move);
        int16_t eval = -negamax(game, depth - 1, -beta, -alpha);
        game.undo();

        if (eval > maxEval) {
            maxEval = eval;
            bestMove = move;
        }
        if (eval > alpha) alpha = eval;
        if (beta <= alpha) break;
    }
    
    TTEntry newEntry;
    newEntry.zobristKey = hash;
    newEntry.depth = depth;
    newEntry.eval = maxEval;
    newEntry.generation = transpositionTable.getGeneration();
    newEntry.bestMove = bestMove;

    if (maxEval <= originalAlpha) newEntry.flag = TTFlag::UPPER_BOUND;
    else if (maxEval >= beta) newEntry.flag = TTFlag::LOWER_BOUND;
    else newEntry.flag = TTFlag::EXACT;

    transpositionTable.add(hash, newEntry);

    return maxEval;
}

int16_t Engine::evaluate(Game& game, GameStateEvaluation state, uint8_t depth) {
    if (state == GameStateEvaluation::CHECKMATE) return -CHECKMATE_VALUE - depth;
    
    // Stalemate / Draw by either fifty move rule, repetition or insufficient material
    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        return 0;
    }

    Board& board = game.getBoard();
    Colour currentColour = game.getCurrentTurn();
    Colour opposingColour = (currentColour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    int16_t eval = pieceValueEvaluation(board, currentColour) - pieceValueEvaluation(board, opposingColour);

    return eval;
}

int16_t Engine::quiescence(Game& game, int16_t alpha, int16_t beta, uint8_t qdepth, GameStateEvaluation state) {
    uint64_t hash = game.getHash();
    TTEntry* entry = quiescenceTranspositionTable.getEntry(hash);
    if (entry && entry->depth >= qdepth) {
        if (entry->flag == TTFlag::EXACT) return entry->eval;
        if (entry->flag == TTFlag::LOWER_BOUND && entry->eval >= beta) return entry->eval;
        if (entry->flag == TTFlag::UPPER_BOUND && entry->eval <= alpha) return entry->eval;
    }

    if (qdepth == 0 || (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK)) {
        return evaluate(game, state, qdepth);
    }

    int16_t currentEval = evaluate(game, state, 0);
    if (currentEval >= beta) return beta;
    if (currentEval > alpha) alpha = currentEval;

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    std::vector<Move> captureMoves = MoveGenerator::legalCaptures(board, colour);
    orderQuiescenceMoves(captureMoves, board);

    Move bestMove;

    for (const Move& move : captureMoves) {
        game.makeMove(move);
        state = game.getCurrentGameStateEvaluation();
        int16_t eval = -quiescence(game, -beta, -alpha, qdepth - 1, state);
        game.undo();

        if (eval >= beta) {
            TTEntry newEntry;
            newEntry.zobristKey = hash;
            newEntry.depth = qdepth;
            newEntry.eval = beta;
            newEntry.generation = quiescenceTranspositionTable.getGeneration();
            newEntry.flag = TTFlag::LOWER_BOUND;
            newEntry.bestMove = move;

            quiescenceTranspositionTable.add(hash, newEntry);

            return beta;
        }
        if (eval > alpha) {
            alpha = eval;
            bestMove = move;
        }
    }

    TTEntry newEntry;
    newEntry.zobristKey = hash;
    newEntry.depth = qdepth;
    newEntry.eval = alpha;
    newEntry.generation = quiescenceTranspositionTable.getGeneration();
    newEntry.flag = TTFlag::EXACT;
    newEntry.bestMove = bestMove;

    quiescenceTranspositionTable.add(hash, newEntry);

    return alpha;
}