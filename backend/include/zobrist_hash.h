#ifndef ZOBRIST_HASH_H
#define ZOBRIST_HASH_H

#include <cstdint>
#include "board/board.h"
#include "game/game_state.h"
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
     * @param oldEnPassantSquare The updated square of the pawn that just moved 2 forward before the move if it exists
     * @param newEnPassantSquare The updated square of the pawn that just moved 2 forward after the move if it exists
     * @param oldCastleRights Castling rights before the move, indexed as [colour][kingside/queenside] with kingside before queenside
     * @param newCastleRights Castling rights after the move, indexed as [colour][kingside/queenside] with kingside before queenside
     * @param playerTurn Turn of player that just made their move
     * @param movedPiece Piece that was last moved
     * @return Updated zobrist hash of the current game state
     * @attention currentHash should be first computed once using computeInitialHash
     */
    uint64_t updateHash(uint64_t currentHash, const Move& move, const std::optional<uint8_t> oldEnPassantSquare,
                        const std::optional<uint8_t> newEnPassantSquare, 
                        const std::array<std::array<bool, 2>, 2> oldCastleRights, 
                        const std::array<std::array<bool, 2>, 2> newCastleRights, 
                        Chess::PieceColour playerTurn, Chess::PieceType movedPiece);
}

#endif // ZOBRIST_HASH_H