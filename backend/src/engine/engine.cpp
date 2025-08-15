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
#include "check/check.h"
#include "move/move_generator.h"
#include "chess_types.h"

#include <iostream>

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::Bitboard;
using Chess::toIndex;

uint64_t probes = 0, hits = 0;

namespace {
    bool disableNullPruning(Board& board, Colour colour) {
        constexpr uint16_t DISABLE_NULL_PRUNING_VALUE = 1300;
        constexpr uint16_t KNIGHT_VALUE = 300;
        constexpr uint16_t BISHOP_VALUE = 300;
        constexpr uint16_t ROOK_VALUE = 500;
        constexpr uint16_t QUEEN_VALUE = 900;
        constexpr Piece pieces[4] = {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
        constexpr uint16_t values[4] = {KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE};

        uint16_t total = 0;
        for (uint8_t i = 0; i < 4; i++) {
            uint8_t count = std::popcount(board.getBitboard(pieces[i], colour));
            total += count * values[i];
        }

        return (total <= DISABLE_NULL_PRUNING_VALUE);
    }
}

Engine::Engine(uint8_t maxDepth, uint8_t quiescenceDepth) : 
    MAX_DEPTH(maxDepth),
    QUIESCENCE_DEPTH(quiescenceDepth),
    transpositionTable(256),
    quiescenceTranspositionTable(256),
    negamaxMoveBuffers(maxDepth + MAX_EXTENSION_COUNT + 1),
    quiescenceMoveBuffers(quiescenceDepth + 1) {

        moveBuffer.reserve(256);
        for (auto& buffer : negamaxMoveBuffers) buffer.reserve(256);
        for (auto& buffer : quiescenceMoveBuffers) buffer.reserve(256);
}

Move Engine::getMove(Game& game) {
    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    Move bestMove;
    maxDepthSearched = 0;

    auto start = std::chrono::steady_clock::now();
    int timeLimit = 2000; // In ms

    auto timeUp = [&]() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= timeLimit;
    };

    probes = 0, hits = 0;

    uint8_t depth;
    for (depth = 1; depth <= MAX_DEPTH; depth++) {
        auto profile_start = std::chrono::high_resolution_clock::now();

        moveBuffer.clear();
        MoveGenerator::pseudoLegalMoves(board, colour, moveBuffer);
        if (bestMove != Move()) {
            Evaluation::orderMoves(moveBuffer, board, 0, colour, &bestMove);
        } else {
            Evaluation::orderMoves(moveBuffer, board, 0, colour);
        }

        int16_t alpha = std::numeric_limits<int16_t>::min() + 1;
        int16_t beta = std::numeric_limits<int16_t>::max();

        int16_t bestEval = std::numeric_limits<int16_t>::min();
        Move currentBest;
        
        for (const Move move : moveBuffer) {
            game.makeMove(move);
            bool inCheck = Check::isInCheck(board, colour);

            // Illegal move
            if (inCheck) {
                game.undo();
                continue;
            }

            GameStateEvaluation newState = game.getCurrentGameStateEvaluation();
            int16_t eval = -negamax(game, depth - 1, -beta, -alpha, newState, timeUp);
            game.undo();

            if (timeUp()) break;

            if (eval > bestEval) {
                bestEval = eval;
                currentBest = move;
            }
            if (eval > alpha) alpha = eval;
        }

        if (timeUp()) break;

        bestMove = currentBest;
        currentEvaluation = (game.getCurrentTurn() == Colour::WHITE) ? bestEval : -bestEval;

        auto profile_end = std::chrono::high_resolution_clock::now();
        std::cout << "Time taken at depth " << static_cast<int>(depth) << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(profile_end - profile_start) << std::endl;
    }

    //std::cout << "Depth achieved: " << static_cast<int>(depth - 1) << std::endl;
    std::cout << "Hit rate: " << 100 * (double)hits/(double)probes << "%" << std::endl;

    Evaluation::clearKillerMoveTable();
    Evaluation::ageHistoryHeuristicsTable();
    transpositionTable.incrementGeneration();
    quiescenceTranspositionTable.incrementGeneration();
    previousMove = bestMove;

    return bestMove;
}

int16_t Engine::negamax(Game& game, int depth, int16_t alpha, int16_t beta, GameStateEvaluation state, 
                        const std::function<bool()>& timeUp, uint8_t ply, int extensionCount, bool allowNullMove) {

    uint64_t hash = game.getHash();
    TTEntry* entry = transpositionTable.getEntry(hash);
    probes++;
    if (entry && entry->depth >= depth) {
        hits++;
        if (entry->flag == TTFlag::EXACT ||
           (entry->flag == TTFlag::LOWER_BOUND && entry->eval >= beta) ||
           (entry->flag == TTFlag::UPPER_BOUND && entry->eval <= alpha)) {

            maxDepthSearched = std::max(maxDepthSearched, ply);
            return entry->eval;
        }
    }

    if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
        maxDepthSearched = std::max(maxDepthSearched, ply);
        return Evaluation::evaluate(game, state, ply);
    }

    if (depth <= 0) return quiescence(game, alpha, beta, QUIESCENCE_DEPTH, state, ply);

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();

    // Null move pruning
    bool inCheck = (state == GameStateEvaluation::CHECK);
    if (allowNullMove && !inCheck && depth >= 3 && !disableNullPruning(board, colour)) {
        game.makeNullMove();
        GameStateEvaluation newState = game.getCurrentGameStateEvaluation();
        const int NULL_MOVE_REDUCTION = (depth >= 6) ? 3 : 2;
        int16_t nullEval = -negamax(game, depth - NULL_MOVE_REDUCTION - 1, -beta, -(beta - 1), newState, timeUp, ply + 1, extensionCount, false);
        game.undoNullMove();

        if (nullEval >= beta) {
            return beta;
        }
    }

    int16_t originalAlpha = alpha;
    int16_t maxEval = std::numeric_limits<int16_t>::min() + 1;
    Move bestMove;

    Move* ttMove = nullptr;
    if (entry && entry->generation == transpositionTable.getGeneration() && entry->depth >= depth) {
        ttMove = &entry->bestMove;
    }

    std::vector<Move>& moves = negamaxMoveBuffers[ply];
    moves.clear();
    MoveGenerator::pseudoLegalMoves(board, colour, moves);
    Evaluation::orderMoves(moves, board, ply, colour, ttMove);

    int moveCount = 0;
    for (const Move move : moves) {
        game.makeMove(move);

        // Illegal move
        if (Check::isInCheck(board, colour)) {
            game.undo();
            continue;
        }

        GameStateEvaluation newState = game.getCurrentGameStateEvaluation();
        uint8_t extension = (newState == GameStateEvaluation::CHECK && extensionCount < MAX_EXTENSION_COUNT) ? 1 : 0;
        int newDepth = depth + extension - 1;

        // Late Move Reduction
        bool doLateMoveReduction = (newState != GameStateEvaluation::CHECK &&
                                    depth >= 3 &&
                                    moveCount >= 4 &&
                                    move.getCapturedPiece() == Move::NO_CAPTURE &&
                                    move.getPromotionPiece() == Move::NO_PROMOTION &&
                                    !Evaluation::isKillerMove(move, ply));

        if (doLateMoveReduction) {
            newDepth--;
            if (moveCount >= 6) newDepth--;
            if (moveCount >= 12) newDepth--;
        }

        int16_t eval = -negamax(game, newDepth, -beta, -alpha, newState, timeUp, ply + 1, extensionCount + extension, allowNullMove);

        // Search again if Late Move Reduction fails
        if (doLateMoveReduction && eval > alpha && eval < beta) {
            eval = -negamax(game, depth - 1 + extension, -beta, -alpha, newState, timeUp, ply + 1, extensionCount + extension, allowNullMove);
        }

        game.undo();
        moveCount++;

        if (timeUp()) return 0;

        if (eval > maxEval) {
            maxEval = eval;
            bestMove = move;
        }
        if (eval > alpha) alpha = eval;
        if (beta <= alpha) {
            // Quiet move
            if (move.getCapturedPiece() == Move::NO_CAPTURE && move.getPromotionPiece() == Move::NO_PROMOTION) {
                Evaluation::addKillerMove(move, ply);
                Evaluation::addHistoryHeuristic(move, board.getPiece(move.getFromSquare()), colour, depth);
            }
            break;
        }
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

int16_t Engine::quiescence(Game& game, int16_t alpha, int16_t beta, uint8_t qdepth, GameStateEvaluation state, uint8_t ply) {
    uint64_t hash = game.getHash();
    TTEntry* entry = quiescenceTranspositionTable.getEntry(hash);
    if (entry && entry->depth >= qdepth) {
        if (entry->flag == TTFlag::EXACT ||
           (entry->flag == TTFlag::LOWER_BOUND && entry->eval >= beta) ||
           (entry->flag == TTFlag::UPPER_BOUND && entry->eval <= alpha)) {

            maxDepthSearched = std::max(maxDepthSearched, ply);
            return entry->eval;
        }
    }

    if (qdepth == 0 || (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK)) {
        maxDepthSearched = std::max(maxDepthSearched, ply);
        return Evaluation::evaluate(game, state, ply);
    }

    int16_t currentEval = Evaluation::evaluate(game, state, ply);

    // Standard pat
    int16_t bestEval = currentEval;
    if (bestEval >= beta) {
        maxDepthSearched = std::max(maxDepthSearched, ply);
        return bestEval;
    }
    if (bestEval > alpha) alpha = bestEval;

    Board& board = game.getBoard();
    Colour colour = game.getCurrentTurn();
    Colour opposingColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;

    std::vector<Move>& moves = quiescenceMoveBuffers[qdepth];
    moves.clear();

    if (state != GameStateEvaluation::CHECK) {
        MoveGenerator::pseudoLegalCaptures(board, colour, moves);
        MoveGenerator::pseudoLegalQueenPromotions(board, colour, moves);
        MoveGenerator::pseudoLegalNonCaptureChecks(board, colour, board.getKingSquare(opposingColour), moves);
    } else {
        MoveGenerator::pseudoLegalMoves(board, colour, moves);
    }
    Evaluation::orderQuiescenceMoves(moves, board);

    int16_t originalAlpha = alpha;
    Move bestMove;

    for (const Move move : moves) {
        // Delta pruning
        uint8_t capturedPiece = move.getCapturedPiece();
        if (capturedPiece != Move::NO_CAPTURE) {
            int16_t capturedValue = Evaluation::getPieceValue(capturedPiece);
            // Capturing cannot raise alpha
            if (currentEval + capturedValue + DELTA_MARGIN <= alpha && state != GameStateEvaluation::CHECK) {
                continue;
            }
        }

        game.makeMove(move);

        // Illegal move
        if (Check::isInCheck(board, colour)) {
            game.undo();
            continue;
        }

        GameStateEvaluation newState = game.getCurrentGameStateEvaluation();
        int16_t eval = -quiescence(game, -beta, -alpha, qdepth - 1, newState, ply + 1);
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

            return eval;
        }
        if (eval > bestEval) {
            bestEval = eval;
            bestMove = move;
        }
        if (eval > alpha) {
            alpha = eval;
        }
    }

    TTEntry newEntry;
    newEntry.zobristKey = hash;
    newEntry.depth = qdepth;
    newEntry.eval = bestEval;
    newEntry.generation = quiescenceTranspositionTable.getGeneration();
    newEntry.bestMove = bestMove;

    if (bestEval <= originalAlpha) newEntry.flag = TTFlag::UPPER_BOUND;
    else if (bestEval >= beta) newEntry.flag = TTFlag::LOWER_BOUND;
    else newEntry.flag = TTFlag::EXACT;

    quiescenceTranspositionTable.add(hash, newEntry);

    return bestEval;
}