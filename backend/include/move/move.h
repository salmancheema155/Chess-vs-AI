#ifndef MOVE_H
#define MOVE_H

#include <optional>
#include "chess_types.h"

namespace ChessMove {
    /**
     * Struct holding data to represent a move
     */
    struct Move {
        Chess::PieceType piece; ///< Piece that is moving
        Chess::PieceColour colour; ///< Colour of piece
        uint8_t fromSquare; ///< Square the piece is moving from
        uint8_t toSquare; ///< Square the piece is moving to
        std::optional<uint8_t> captureSquare = std::nullopt; ///< Piece that is captured from this move
    };

    /**
     * @brief Creates instance of Move struct
     * @param piece Piece to move
     * @param colour Colour of piece
     * @param fromSquare Square that the piece moves from
     * @param toSquare Square that the piece moves to
     * @param captureSquare Piece that is captured from this move
     * @return Move instance representing the move
     */
    inline constexpr Move makeMove(Chess::PieceType piece, Chess::PieceColour colour, 
                            uint8_t fromSquare, uint8_t toSquare, 
                            std::optional<uint8_t> captureSquare = std::nullopt) {
        return {
            .piece = piece,
            .colour = colour,
            .fromSquare = fromSquare,
            .toSquare = toSquare,
            .captureSquare = captureSquare
        };
    }
}

#endif // MOVE_H