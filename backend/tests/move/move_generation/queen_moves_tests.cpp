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

TEST(legalQueenGeneratorTest, board1) {
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

    // White queen on d1
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("d1");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::QUEEN, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("c1")),
                                            Move(square, algebraicToSquare("b1")),
                                            Move(square, algebraicToSquare("e1")),
                                            Move(square, algebraicToSquare("d2")),
                                            Move(square, algebraicToSquare("d3")),
                                            Move(square, algebraicToSquare("c2")),
                                            Move(square, algebraicToSquare("b3")),
                                            Move(square, algebraicToSquare("a4")),
                                            Move(square, algebraicToSquare("e2"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black queen on d8
    colour = Colour::BLACK;
    square = algebraicToSquare("d8");
    legalMoves = MoveGenerator::legalMoves(b, Piece::QUEEN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("e8")),
                            Move(square, algebraicToSquare("d7")),
                            Move(square, algebraicToSquare("e7")),
                            Move(square, algebraicToSquare("f6")),
                            Move(square, algebraicToSquare("g5")),
                            Move(square, algebraicToSquare("h4"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(legalQueenGeneratorTest, board2) {
    Board b;

    b.setCustomBoardState("r1rq2kn/2p3pp/p3P2R/1pB1p2n/6Q1/2N1P2P/PPP3P1/5RK1 w - - 2 25");
    //printBoard(b); // View visual board for test

    // White queen on g4
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("g4");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::QUEEN, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("h4")),
                                            Move(square, algebraicToSquare("f4")),
                                            Move(square, algebraicToSquare("e4")),
                                            Move(square, algebraicToSquare("d4")),
                                            Move(square, algebraicToSquare("c4")),
                                            Move(square, algebraicToSquare("b4")),
                                            Move(square, algebraicToSquare("a4")),
                                            Move(square, algebraicToSquare("g3")),
                                            Move(square, algebraicToSquare("g5")),
                                            Move(square, algebraicToSquare("g6")),
                                            Move(square, algebraicToSquare("g7"), toIndex(Piece::PAWN)),
                                            Move(square, algebraicToSquare("h5"), toIndex(Piece::KNIGHT)),
                                            Move(square, algebraicToSquare("f5")),
                                            Move(square, algebraicToSquare("f3")),
                                            Move(square, algebraicToSquare("e2")),
                                            Move(square, algebraicToSquare("d1"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black queen on d8
    colour = Colour::BLACK;
    square = algebraicToSquare("d8");
    legalMoves = MoveGenerator::legalMoves(b, Piece::QUEEN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("e8")),
                            Move(square, algebraicToSquare("f8")),
                            Move(square, algebraicToSquare("d7")),
                            Move(square, algebraicToSquare("d6")),
                            Move(square, algebraicToSquare("d5")),
                            Move(square, algebraicToSquare("d4")),
                            Move(square, algebraicToSquare("d3")),
                            Move(square, algebraicToSquare("d2")),
                            Move(square, algebraicToSquare("d1")),
                            Move(square, algebraicToSquare("e7")),
                            Move(square, algebraicToSquare("f6")),
                            Move(square, algebraicToSquare("g5")),
                            Move(square, algebraicToSquare("h4"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}