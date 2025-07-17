#include <array>
#include <vector>
#include <cstdint>
#include "chess_types.h"
#include "board_utilities.h"

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::toIndex;

static const std::array<std::array<std::vector<uint8_t>, 2>, 6> expectedInitialStartingSquares = {{
    // Pawns
    {{
        {8, 9, 10, 11, 12, 13, 14, 15},
        {48, 49, 50, 51, 52, 53, 54, 55}
    }},
    // Knights
    {{
        {1, 6},
        {57, 62}
    }},
    // Bishops
    {{
        {2, 5},
        {58, 61}
    }},
    // Rooks
    {{
        {0, 7},
        {56, 63}
    }},
    // Queen
    {{
        {3},
        {59}
    }},
    // King
    {{
        {4},
        {60}
    }}
}};

std::vector<uint8_t> BoardUtils::getExpectedInitialStartingSquares(Piece piece, Colour colour) {
    return expectedInitialStartingSquares[toIndex(piece)][toIndex(colour)];
}