#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <cstdint>
#include <optional>
#include "board/board.h"
#include "move/move.h"
#include "chess_types.h"

/**
 * Holds data about the current game state
 */
struct GameState {
    Board board; ///< Board object representing the current board state
    Chess::PieceColour playerTurn; ///< Current player's turn

    uint16_t halfMoveClock; ///< Number of half moves elapsed since a pawn move or capture
    uint16_t fullMoves; ///< Number of moves elapsed since the start of the game starting at 1 and incremented after black's move

    uint64_t hash; ///< Zobrist hash of current board state
};

#endif // GAME_STATE_H