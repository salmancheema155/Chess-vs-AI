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
#include "chess_types.h"

using Chess::Bitboard;
using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;

namespace {
    // Sorts moves in ascending order by the .toSquare
    void sortMoves(std::vector<Move>& legalMoves) {
        std::sort(legalMoves.begin(), legalMoves.end(), [](const Move& a, const Move& b) {
            return a.getToSquare() < b.getToSquare();
        });
    }
}

TEST(legalKnightGeneratorTest, board1) {
    Board b;
    /*
    1. e4 e5
    2. Nf3 Nc6
    3. Bc4 Nf6
    4. Nc3 Bc5
    5. d3 O-O
    6. Bg5 h6
    7. Bxf6 Qxf6
    8. Nd5 Qd8
    9. O-O d6
    10. c3 a5
    11. d4
    */

    b.setCustomBoardState("r1bq1rk1/1pp2pp1/2np3p/p1bNp3/2BPP3/2P2N2/PP3PPP/R2Q1RK1 b - - 0 11");
    //printBoard(b); // View visual board for test

    // White knight on d5
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("d5");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::KNIGHT, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("c7"), toIndex(Piece::PAWN)),
                                            Move(square, algebraicToSquare("e7")),
                                            Move(square, algebraicToSquare("f6")),
                                            Move(square, algebraicToSquare("f4")),
                                            Move(square, algebraicToSquare("e3")),
                                            Move(square, algebraicToSquare("b4")),
                                            Move(square, algebraicToSquare("b6"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // White knight on f3
    colour = Colour::WHITE;
    square = algebraicToSquare("f3");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KNIGHT, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("e5"), toIndex(Piece::PAWN)),
                            Move(square, algebraicToSquare("g5")),
                            Move(square, algebraicToSquare("h4")),
                            Move(square, algebraicToSquare("e1")),
                            Move(square, algebraicToSquare("d2"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black knight on c6
    colour = Colour::BLACK;
    square = algebraicToSquare("c6");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KNIGHT, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("b8")),
                            Move(square, algebraicToSquare("e7")),
                            Move(square, algebraicToSquare("d4"), toIndex(Piece::PAWN)),
                            Move(square, algebraicToSquare("b4")),
                            Move(square, algebraicToSquare("a7"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(legalKnightGeneratorTest, board2) {
    Board b;

    b.setCustomBoardState("r1rq2kn/2p3pp/p3P2R/1pB1p2n/6Q1/2N1P2P/PPP3P1/5RK1 w - - 2 25");
    //printBoard(b); // View visual board for test

    // White knight on c3
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("c3");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::KNIGHT, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("b5"), toIndex(Piece::PAWN)),
                                            Move(square, algebraicToSquare("d5")),
                                            Move(square, algebraicToSquare("e4")),
                                            Move(square, algebraicToSquare("e2")),
                                            Move(square, algebraicToSquare("d1")),
                                            Move(square, algebraicToSquare("b1")),
                                            Move(square, algebraicToSquare("a4"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black knight on h5
    colour = Colour::BLACK;
    square = algebraicToSquare("h5");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KNIGHT, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("g3")),
                            Move(square, algebraicToSquare("f4")),
                            Move(square, algebraicToSquare("f6"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black knight on h8
    colour = Colour::BLACK;
    square = algebraicToSquare("h8");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KNIGHT, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("f7")),
                            Move(square, algebraicToSquare("g6"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}