#ifndef NDEBUG // Only compile in debug builds

#include <iostream>
#include "board/board.h"
#include "chess_types.h"

using Chess::toIndex;

namespace {
    constexpr const char* white_pawn = "♙";
    constexpr const char* white_knight = "♘";
    constexpr const char* white_bishop = "♗";
    constexpr const char* white_rook = "♖";
    constexpr const char* white_queen = "♕";
    constexpr const char* white_king = "♔";

    constexpr const char* black_pawn = "♟";
    constexpr const char* black_knight = "♞";
    constexpr const char* black_bishop = "♝";
    constexpr const char* black_rook = "♜";
    constexpr const char* black_queen = "♛";
    constexpr const char* black_king = "♚";

    constexpr const char* pieces[2][6] = {{white_pawn, white_knight, white_bishop, white_rook, white_queen, white_king}, 
                                        {black_pawn, black_knight, black_bishop, black_rook, black_queen, black_king}};

    void printSquare(const Board& board, uint8_t square) {
        auto [piece, colour] = board.getPieceAndColour(square);
        if (piece) { // piece exists at this square
            std::cout << pieces[toIndex(*colour)][toIndex(*piece)];
        } else {
            std::cout << " ";
        }
    }
}

void printBoard(const Board& board, char delimiter) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 7; col++) {
            printSquare(board, 8 * row + col);
            std::cout << delimiter;
        }
        printSquare(board, 8 * row + 7);
        std::cout << "\n";
    }
}

#endif // NDEBUG