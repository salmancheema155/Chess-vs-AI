#ifndef MOVE_DEBUG_H
#define MOVE_DEBUG_H

#include <ostream>
#include <vector>
#include <string>
#include <cassert>
#include "board/board.h"
#include "move/move.h"
#include "chess_types.h"

namespace {
    inline const char* pieceMap[] = {"PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"};
    inline const char* colourMap[] = {"WHITE", "BLACK"};
    inline const char* castlingMap[] = {"KINGSIDE", "QUEENSIDE"};
    inline const char* enPassantMap[] = {"FALSE", "TRUE"};

    uint8_t algebraicToSquare(std::string algString) {
        return (algString[0] - 'a') + 8 * (algString[1] - '1');
    }

    std::string squareToAlgebraic(uint8_t square) {
        assert(square < 64 && "Square must be between 0-63");
        uint8_t file = Board::getFile(square);
        uint8_t rank = Board::getRank(square);

        char fileChar = 'a' + file;
        char rankChar = '1' + rank;

        return std::string() + fileChar + rankChar;
    }
}

inline std::ostream& operator<<(std::ostream& os, const Move& move) {
    os <<
    "{from square: " << squareToAlgebraic(move.getFromSquare()) << ", " <<
    "to square: " << squareToAlgebraic(move.getToSquare()) << ", ";

    if (move.getCapturedPiece() != Move::NO_CAPTURE) {
        os << "captured piece: " << pieceMap[move.getCapturedPiece()] << ", ";
    } else {
        os << "captured piece: NONE, ";
    }

    if (move.getPromotionPiece() != Move::NO_PROMOTION) {
        os << "promotion piece: " << pieceMap[move.getPromotionPiece()] << ", ";
    } else {
        os << "promotion piece: NONE, ";
    }


    if (move.getCastling() != Move::NO_CASTLE) {
        os << "castling: " << castlingMap[move.getCastling()] << ", ";
    } else {
        os << "castling: NONE, ";
    }
    
    os << "en passant: " << enPassantMap[move.getEnPassant()] << "}";

    return os;
}

#endif // MOVE_DEBUG_H