#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <cstdint>
#include <array>
#include <optional>
#include "board/board.h"
#include "move/move.h"
#include "chess_types.h"

/**
 * Holds data about the current game state
 */
struct GameState {
    Chess::PieceColour playerTurn; ///< Current player's turn
    std::optional<uint8_t> enPassantSquare; ///< Current square of the pawn that just moved 2 forward if it exists

    std::array<std::array<bool, 2>, 2> castleRights; ///< Indexed as [colour][kingside/queenside] with kingside before queenside.

    uint16_t halfMoveClock; ///< Number of half moves elapsed since a pawn move or capture
    uint16_t fullMoves; ///< Number of moves elapsed since the start of the game starting at 1 and incremented after black's move

    uint64_t hash; ///< Zobrist hash of current board state
};

GameState createGameState(Chess::PieceColour playerTurn, std::optional<uint8_t> enPassantSquare, 
                            const std::array<std::array<bool, 2>, 2>& castleRights, uint16_t halfMoveClock, uint16_t fullMoves, uint64_t hash) {

    return {
        .playerTurn = playerTurn,
        .enPassantSquare = enPassantSquare,
        .castleRights = castleRights,
        .halfMoveClock = halfMoveClock,
        .fullMoves = fullMoves,
        .hash = hash
    };
}

#endif // GAME_STATE_H