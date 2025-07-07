#include <gtest/gtest.h>
#include <cstdint>
#include <optional>
#include <array>
#include <vector>
#include <algorithm>
#include <ostream>
#include <bitset>
#include "board/board.h"
#include "move/move.h"
#include "move/move_generator.h"
#include "tests/move/move_debug.h"
#include "tests/board/board_debug.h"
#include "check/check.h"
#include "chess_types.h"

using Chess::Bitboard;
using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;

TEST(checkTest, board1) {
    Board b;

    b.setCustomBoardState("1n1qk1r1/8/8/1BPp1p1p/1P3P1P/4P3/8/3Q1KR1 b - - 8 34");
    //printBoard(b); // View visual board for test

    // Black king on e8
    Colour colour = Colour::BLACK;
    ASSERT_TRUE(Check::isInCheck(b, colour));

    // White king on f1
    colour = Colour::WHITE;
    ASSERT_FALSE(Check::isInCheck(b, colour));
}

TEST(checkTest, board2) {
    Board b;

    b.setCustomBoardState("1n1qk1r1/8/8/2Pp1p1p/1PB2P1P/4P3/8/3Q1KR1 b - - 8 35");
    //printBoard(b); // View visual board for test

    // Black king on e8
    Colour colour = Colour::BLACK;
    ASSERT_FALSE(Check::isInCheck(b, colour));

    // White king on f1
    colour = Colour::WHITE;
    ASSERT_FALSE(Check::isInCheck(b, colour));
}

TEST(checkTest, board3) {
    Board b;

    b.setCustomBoardState("1n4r1/4k3/8/qPPp1p1p/2B2P1P/4P3/3K4/3Q2R1 w - - 7 45");
    //printBoard(b); // View visual board for test

    // Black king on e7
    Colour colour = Colour::BLACK;
    ASSERT_FALSE(Check::isInCheck(b, colour));

    // White king on d2
    colour = Colour::WHITE;
    ASSERT_TRUE(Check::isInCheck(b, colour));
}

TEST(checkTest, board4) {
    Board b;

    b.setCustomBoardState("1n4r1/4N3/q1k5/2Pp1p1p/1PB2P1P/4P3/1K6/3Q2R1 w - - 10 56");
    //printBoard(b); // View visual board for test

    // Black king on c6
    Colour colour = Colour::BLACK;
    ASSERT_TRUE(Check::isInCheck(b, colour));

    // White king on b2
    colour = Colour::WHITE;
    ASSERT_FALSE(Check::isInCheck(b, colour));
}

TEST(checkTest, board5) {
    Board b;

    b.setCustomBoardState("1n2N1r1/8/q1k2b2/2Pp1p1p/1PB2P1P/4P3/1K6/3Q2R1 w - - 3 47");
    //printBoard(b); // View visual board for test

    // Black king on c6
    Colour colour = Colour::BLACK;
    ASSERT_FALSE(Check::isInCheck(b, colour));

    // White king on b2
    colour = Colour::WHITE;
    ASSERT_TRUE(Check::isInCheck(b, colour));
}