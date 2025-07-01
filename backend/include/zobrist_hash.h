#ifndef ZOBRIST_HASH_H
#define ZOBRIST_HASH_H

#include <cstdint>
#include "board/board.h"
#include "game_state.h"
#include "chess_types.h"

namespace Zobrist {
    /**
     * @brief Computes the zobrist hash of the game state
     * @param board Board object representing current board state
     * @param playerTurn Turn of player which corresponds to current game state
     * @return Zobrist hash of the current game state
     * @note This function should only be used to compute the initial zobrist hash
     * Updates to the game state hash should be computed more efficiently using updateHash
     */
    uint64_t computeInitialHash(const Board& board, const Chess::PieceColour playerTurn);

    /**
     * @brief Updates the zobrist hash of the game state
     * @param currentHash Zobrist hash computed from the previous board state
     * @param move Move object representing the current move
     * @param oldBoard Board object respresenting the board state before making the move
     * @param newBoard Board object representing the board state after making the move
     * @return Updated zobrist hash of the current game state
     * @note currentHash should be first computed once using computeInitialHash
     */
    uint64_t updateHash(uint64_t currentHash, const Move& move, const Board& oldBoard, const Board& newBoard);
}

#endif // ZOBRIST_HASH_H