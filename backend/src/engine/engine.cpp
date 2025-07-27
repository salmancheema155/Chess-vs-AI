#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <functional>
#include <bit>
#include <chrono>
#include "engine/engine.h"
#include "engine/transposition_table.h"
#include "engine/evaluation.h"
#include "board/board.h"
#include "move/move.h"
#include "game/game.h"
#include "move/move_generator.h"
#include "chess_types.h"

#include <iostream>

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::Bitboard;
using Chess::toIndex;

uint64_t probes = 0, hits = 0;
uint64_t qprobes = 0, qhits = 0;

Engine::Engine(uint8_t maxDepth, uint8_t quiescenceDepth) : 
    MAX_DEPTH(maxDepth),
    QUIESCENCE_DEPTH(quiescenceDepth),
    transpositionTable(512),
    quiescenceTranspositionTable(256),
    negamaxMoveBuffers(maxDepth),
    quiescenceMoveBuffers(quiescenceDepth + 1) {

        moveBuffer.reserve(256);
        for (auto& buffer : negamaxMoveBuffers) buffer.reserve(256);
        for (auto& buffer : quiescenceMoveBuffers) buffer.reserve(128);
}

Move Engine::getMove(Game& game) {
    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    Move bestMove;

    auto start = std::chrono::steady_clock::now();
    int timeLimit = 3000; // In ms

    auto timeUp = [&]() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= timeLimit;
    };

    probes = 0;
    hits = 0;
    qprobes = 0;
    qhits = 0;

    uint8_t depth;
    for (depth = 1; depth <= MAX_DEPTH; depth++) {
        if (timeUp()) break;
        
        moveBuffer.clear();
        MoveGenerator::legalMoves(board, colour, moveBuffer);
        Evaluation::orderMoves(moveBuffer, board);

        int16_t bestEval = std::numeric_limits<int16_t>::min();
        Move currentBest;
        
        for (const Move move : moveBuffer) {
            if (timeUp()) break;
            game.makeMove(move);
            int16_t eval = -negamax(game, depth - 1, std::numeric_limits<int16_t>::min() + 1, std::numeric_limits<int16_t>::max(), timeUp);
            game.undo();

            if (eval > bestEval) {
                bestEval = eval;
                currentBest = move;
            }
        }

        if (!timeUp()) bestMove = currentBest;
        if (bestMove == Move()) bestMove = moveBuffer[0];
    }

    transpositionTable.incrementGeneration();
    quiescenceTranspositionTable.incrementGeneration();

    std::cout << "Probes: " << probes << std::endl;
    std::cout << "Hits: " << hits << std::endl;
    std::cout << "Hit rate: " << 100 * (double)hits / (double)probes << "%" << std::endl;

    std::cout << "Depth Completed: " << static_cast<int>(depth - 1) << std::endl;

    std::cout << "qProbes: " << qprobes << std::endl;
    std::cout << "qHits: " << qhits << std::endl;
    std::cout << "qHit rate: " << 100 * (double)qhits / (double)qprobes << "%" << std::endl;

    return bestMove;
}

int16_t Engine::negamax(Game& game, uint8_t depth, int16_t alpha, int16_t beta, const std::function<bool()>& timeUp) {
    if (timeUp()) return alpha;

    uint64_t hash = game.getHash();
    TTEntry* entry = transpositionTable.getEntry(hash);
    probes++;
    if (entry && entry->depth >= depth) {
        hits++;
        if (entry->flag == TTFlag::EXACT) return entry->eval;
        if (entry->flag == TTFlag::LOWER_BOUND && entry->eval >= beta) return entry->eval;
        if (entry->flag == TTFlag::UPPER_BOUND && entry->eval <= alpha) return entry->eval;
    }

    GameStateEvaluation state = game.getCurrentGameStateEvaluation();
    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        return Evaluation::evaluate(game, state, depth);
    }

    if (depth == 0) return quiescence(game, alpha, beta, QUIESCENCE_DEPTH, state);

    int16_t originalAlpha = alpha;
    int16_t maxEval = std::numeric_limits<int16_t>::min() + 1;
    Move bestMove;

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();

    Move* ttMove = nullptr;
    if (entry && entry->generation == transpositionTable.getGeneration() && entry->depth >= depth) {
        ttMove = &entry->bestMove;
    }

    std::vector<Move>& moves = negamaxMoveBuffers[depth];
    moves.clear();
    MoveGenerator::legalMoves(board, colour, moves);
    Evaluation::orderMoves(moves, board, ttMove);

    for (const Move move : moves) {
        if (timeUp()) return alpha;
        game.makeMove(move);
        int16_t eval = -negamax(game, depth - 1, -beta, -alpha, timeUp);
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

int16_t Engine::quiescence(Game& game, int16_t alpha, int16_t beta, uint8_t qdepth, GameStateEvaluation state) {
    uint64_t hash = game.getHash();
    TTEntry* entry = quiescenceTranspositionTable.getEntry(hash);
    qprobes++;
    if (entry && entry->depth >= qdepth) {
        qhits++;
        if (entry->flag == TTFlag::EXACT) return entry->eval;
        if (entry->flag == TTFlag::LOWER_BOUND && entry->eval >= beta) return entry->eval;
        if (entry->flag == TTFlag::UPPER_BOUND && entry->eval <= alpha) return entry->eval;
    }

    if (qdepth == 0 || (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK)) {
        return Evaluation::evaluate(game, state, qdepth);
    }

    int16_t currentEval = Evaluation::evaluate(game, state, 0);
    if (currentEval >= beta) return beta;
    if (currentEval > alpha) alpha = currentEval;

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();

    std::vector<Move>& captureMoves = quiescenceMoveBuffers[qdepth];
    captureMoves.clear();
    MoveGenerator::legalCaptures(board, colour, captureMoves);

    Evaluation::orderQuiescenceMoves(captureMoves, board);

    Move bestMove;

    for (const Move move : captureMoves) {
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