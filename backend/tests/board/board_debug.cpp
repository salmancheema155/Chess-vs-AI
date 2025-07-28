#ifndef NDEBUG // Only compile in debug builds

#include <iostream>
#include <cassert>
#include <array>
#include <string>
#include <cstdint>
#include <cctype>
#include "board/board.h"
#include "chess_types.h"

using Chess::toIndex;
using Chess::Castling;
using Chess::Bitboard;
using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;

namespace {
    constexpr const char* white_pawn = "P";
    constexpr const char* white_knight = "N";
    constexpr const char* white_bishop = "B";
    constexpr const char* white_rook = "R";
    constexpr const char* white_queen = "Q";
    constexpr const char* white_king = "K";

    constexpr const char* black_pawn = "p";
    constexpr const char* black_knight = "n";
    constexpr const char* black_bishop = "b";
    constexpr const char* black_rook = "r";
    constexpr const char* black_queen = "q";
    constexpr const char* black_king = "k";

    constexpr const char* pieces[2][6] = {{white_pawn, white_knight, white_bishop, white_rook, white_queen, white_king}, 
                                        {black_pawn, black_knight, black_bishop, black_rook, black_queen, black_king}};

    void printSquare(const Board& board, uint8_t square) {
        auto [piece, colour] = board.getPieceAndColour(square);
        if (piece != Piece::NONE) { // piece exists at this square
            std::cout << pieces[toIndex(colour)][toIndex(piece)];
        } else {
            std::cout << " ";
        }
    }
}

void printBoard(const Board& board, char delimiter) {
    for (int row = 7; row >= 0; row--) {
        for (int col = 0; col < 8; col++) {
            printSquare(board, 8 * row + col);
            std::cout << delimiter;
        }
        std::cout << "\n";
    }
}

#endif // NDEBUG