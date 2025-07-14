#ifndef MOVE_INFO_H
#define MOVE_INFO_H

#include <cstdint>
#include "move/move.h"

struct MoveInfo {
    Move move;
    uint8_t movedPiece;
    uint8_t movedPieceColour;
    uint8_t capturedPiece;
    uint8_t capturedPieceColour;
};

#endif // MOVE_INFO_H