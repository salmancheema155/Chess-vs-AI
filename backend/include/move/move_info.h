#ifndef MOVE_INFO_H
#define MOVE_INFO_H

#include <cstdint>
#include "move/move.h"

/**
 * Struct holding more detailed information about a move
 * See move.h for more detail on move flags
 * 
 * Flags are defined as follows:
 * - movedPiece:
 *      - PAWN = 0
 *      - KNIGHT = 1
 *      - BISHOP = 2
 *      - ROOK = 3
 *      - QUEEN = 4
 *      - KING = 5
 * 
 * - capturedPiece:
 *      - PAWN = 0
 *      - KNIGHT = 1
 *      - BISHOP = 2
 *      - ROOK = 3
 *      - QUEEN = 4
 *      - NONE = 6
 * 
 * - movedPieceColour:
 *      - WHITE = 0
 *      - BLACK = 1
 * 
 * - capturedPieceColour:
 *      - WHITE = 0
 *      - BLACK = 1
 *      - NONE = 2
 */
struct MoveInfo {
    Move move; ///< Standard move object
    uint8_t movedPiece; ///< Flag denoting which piece moved
    uint8_t movedPieceColour; ///< Flag denoting the colour of the piece which moved
    uint8_t capturedPiece; ///< Flag denoting what piece was captured
    uint8_t capturedPieceColour; ///< Flag denoting the colour of the piece which moved
};

#endif // MOVE_INFO_H