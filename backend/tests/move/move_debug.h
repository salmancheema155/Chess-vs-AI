#ifndef MOVE_DEBUG_H
#define MOVE_DEBUG_H

#include <ostream>
#include <vector>
#include "move/move.h"
#include "chess_types.h"

namespace {
    inline const char* pieceMap[] = {"PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"};
    inline const char* colourMap[] = {"WHITE", "BLACK"};
}

namespace ChessMove {
    inline std::ostream& operator<<(std::ostream& os, const ChessMove::Move& move) {
        os <<
        "{Piece: " << pieceMap[Chess::toIndex(move.piece)] << ", " <<
        "Colour: " << colourMap[Chess::toIndex(move.colour)] << ", " <<
        "fromSquare: " << static_cast<int>(move.fromSquare) << ", " <<
        "toSquare: " << static_cast<int>(move.toSquare) << ", ";

        if (move.captureSquare) {
            os << "captureSquare: " << static_cast<int>(*move.captureSquare);
        } else {
            os << "captureSquare: NONE";
        }

        os << "}";

        return os;
    }
}

#endif // MOVE_DEBUG_H