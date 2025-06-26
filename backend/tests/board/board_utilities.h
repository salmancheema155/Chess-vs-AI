#ifndef BOARD_UTILITIES_H
#define BOARD_UTILITIES_H

#include <vector>
#include <cstdint>
#include "chess_types.h"

namespace BoardUtils {
    std::vector<uint8_t> getExpectedInitialStartingSquares(Chess::PieceType piece, Chess::PieceColour colour);
}

#endif // BOARD_UTILITIES_H