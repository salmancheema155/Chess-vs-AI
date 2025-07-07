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

TEST(promotionMoveTest, board1) {
    Board b;

    b.setCustomBoardState("3q1rk1/P4ppp/2np4/1p3b2/2NP4/4B3/5PPP/Q4RK1 w - - 0 25");
    //printBoard(b); // View visual board for test

    // White pawn on a7
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("a7");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("a8"), Move::NO_CAPTURE, toIndex(Piece::KNIGHT)),
                                            Move(square, algebraicToSquare("a8"), Move::NO_CAPTURE, toIndex(Piece::BISHOP)),
                                            Move(square, algebraicToSquare("a8"), Move::NO_CAPTURE, toIndex(Piece::ROOK)),
                                            Move(square, algebraicToSquare("a8"), Move::NO_CAPTURE, toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(promotionMoveTest, board2) {
    Board b;

    b.setCustomBoardState("1n1qk1r1/PPP1P1PP/8/8/8/8/ppp1p1pp/RNBQKBN1 w Q - 1 59");
    //printBoard(b); // View visual board for test

    // White pawn on a7
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("a7");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("a8"), Move::NO_CAPTURE, toIndex(Piece::KNIGHT)),
                                            Move(square, algebraicToSquare("a8"), Move::NO_CAPTURE, toIndex(Piece::BISHOP)),
                                            Move(square, algebraicToSquare("a8"), Move::NO_CAPTURE, toIndex(Piece::ROOK)),
                                            Move(square, algebraicToSquare("a8"), Move::NO_CAPTURE, toIndex(Piece::QUEEN)),
                                            Move(square, algebraicToSquare("b8"), toIndex(Piece::KNIGHT), toIndex(Piece::KNIGHT)),
                                            Move(square, algebraicToSquare("b8"), toIndex(Piece::KNIGHT), toIndex(Piece::BISHOP)),
                                            Move(square, algebraicToSquare("b8"), toIndex(Piece::KNIGHT), toIndex(Piece::ROOK)),
                                            Move(square, algebraicToSquare("b8"), toIndex(Piece::KNIGHT), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // White pawn on b7
    colour = Colour::WHITE;
    square = algebraicToSquare("b7");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // White pawn on c7
    colour = Colour::WHITE;
    square = algebraicToSquare("c7");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("c8"), Move::NO_CAPTURE, toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("c8"), Move::NO_CAPTURE, toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("c8"), Move::NO_CAPTURE, toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("c8"), Move::NO_CAPTURE, toIndex(Piece::QUEEN)),
                            Move(square, algebraicToSquare("b8"), toIndex(Piece::KNIGHT), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("b8"), toIndex(Piece::KNIGHT), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("b8"), toIndex(Piece::KNIGHT), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("b8"), toIndex(Piece::KNIGHT), toIndex(Piece::QUEEN)),
                            Move(square, algebraicToSquare("d8"), toIndex(Piece::QUEEN), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("d8"), toIndex(Piece::QUEEN), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("d8"), toIndex(Piece::QUEEN), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("d8"), toIndex(Piece::QUEEN), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // White pawn on e7
    colour = Colour::WHITE;
    square = algebraicToSquare("e7");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("d8"), toIndex(Piece::QUEEN), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("d8"), toIndex(Piece::QUEEN), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("d8"), toIndex(Piece::QUEEN), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("d8"), toIndex(Piece::QUEEN), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // White pawn on g7
    colour = Colour::WHITE;
    square = algebraicToSquare("g7");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // White pawn on h7
    colour = Colour::WHITE;
    square = algebraicToSquare("h7");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("h8"), Move::NO_CAPTURE, toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("h8"), Move::NO_CAPTURE, toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("h8"), Move::NO_CAPTURE, toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("h8"), Move::NO_CAPTURE, toIndex(Piece::QUEEN)),
                            Move(square, algebraicToSquare("g8"), toIndex(Piece::ROOK), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("g8"), toIndex(Piece::ROOK), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("g8"), toIndex(Piece::ROOK), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("g8"), toIndex(Piece::ROOK), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black pawn on a2
    colour = Colour::BLACK;
    square = algebraicToSquare("a2");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("b1"), toIndex(Piece::KNIGHT), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("b1"), toIndex(Piece::KNIGHT), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("b1"), toIndex(Piece::KNIGHT), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("b1"), toIndex(Piece::KNIGHT), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black pawn on b2
    colour = Colour::BLACK;
    square = algebraicToSquare("b2");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {
                            Move(square, algebraicToSquare("a1"), toIndex(Piece::ROOK), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("a1"), toIndex(Piece::ROOK), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("a1"), toIndex(Piece::ROOK), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("a1"), toIndex(Piece::ROOK), toIndex(Piece::QUEEN)),
                            Move(square, algebraicToSquare("c1"), toIndex(Piece::BISHOP), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("c1"), toIndex(Piece::BISHOP), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("c1"), toIndex(Piece::BISHOP), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("c1"), toIndex(Piece::BISHOP), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black pawn on c2
    colour = Colour::BLACK;
    square = algebraicToSquare("c2");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("b1"), toIndex(Piece::KNIGHT), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("b1"), toIndex(Piece::KNIGHT), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("b1"), toIndex(Piece::KNIGHT), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("b1"), toIndex(Piece::KNIGHT), toIndex(Piece::QUEEN)),
                            Move(square, algebraicToSquare("d1"), toIndex(Piece::QUEEN), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("d1"), toIndex(Piece::QUEEN), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("d1"), toIndex(Piece::QUEEN), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("d1"), toIndex(Piece::QUEEN), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black pawn on e2
    colour = Colour::BLACK;
    square = algebraicToSquare("e2");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("d1"), toIndex(Piece::QUEEN), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("d1"), toIndex(Piece::QUEEN), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("d1"), toIndex(Piece::QUEEN), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("d1"), toIndex(Piece::QUEEN), toIndex(Piece::QUEEN)),
                            Move(square, algebraicToSquare("f1"), toIndex(Piece::BISHOP), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("f1"), toIndex(Piece::BISHOP), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("f1"), toIndex(Piece::BISHOP), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("f1"), toIndex(Piece::BISHOP), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black pawn on g2
    colour = Colour::BLACK;
    square = algebraicToSquare("g2");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("f1"), toIndex(Piece::BISHOP), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("f1"), toIndex(Piece::BISHOP), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("f1"), toIndex(Piece::BISHOP), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("f1"), toIndex(Piece::BISHOP), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black pawn on h2
    colour = Colour::BLACK;
    square = algebraicToSquare("h2");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("h1"), Move::NO_CAPTURE, toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("h1"), Move::NO_CAPTURE, toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("h1"), Move::NO_CAPTURE, toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("h1"), Move::NO_CAPTURE, toIndex(Piece::QUEEN)),
                            Move(square, algebraicToSquare("g1"), toIndex(Piece::KNIGHT), toIndex(Piece::KNIGHT)),
                            Move(square, algebraicToSquare("g1"), toIndex(Piece::KNIGHT), toIndex(Piece::BISHOP)),
                            Move(square, algebraicToSquare("g1"), toIndex(Piece::KNIGHT), toIndex(Piece::ROOK)),
                            Move(square, algebraicToSquare("g1"), toIndex(Piece::KNIGHT), toIndex(Piece::QUEEN))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}