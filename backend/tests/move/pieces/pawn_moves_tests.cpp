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

TEST(legalPawnGeneratorTest, board1) {
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

    // White pawn on d4
    Colour colour = Colour::WHITE;
    uint8_t square = 27;
    std::vector<Move> legalMoves;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    std::vector<Move> expectedLegalMoves = {Move(square, square + 7, toIndex(Piece::BISHOP)), 
                                            Move(square, square + 9, toIndex(Piece::PAWN))};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // White pawn on c3
    colour = Colour::WHITE;
    square = 18;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // White pawn on a2
    colour = Colour::WHITE;
    square = 8;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {Move(square, square + 8), 
                            Move(square, square + 16)};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // White pawn on h2
    colour = Colour::WHITE;
    square = 15;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {Move(square, square + 8), 
                            Move(square, square + 16)};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // Black pawn on d6
    colour = Colour::BLACK;
    square = 43;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // Black pawn on e5
    colour = Colour::BLACK;
    square = 36;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {Move(square, square - 9, toIndex(Piece::PAWN))};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // Black pawn on a5
    colour = Colour::BLACK;
    square = 32;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {Move(square, square - 8)};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // black pawn on b7
    colour = Colour::BLACK;
    square = 49;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {Move(square, square - 16), 
                            Move(square, square - 8)};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();
}

TEST(legalPawnGeneratorTest, board2) {
    Board b;

    b.setCustomBoardState("r1rq2kn/2p3pp/p3P2R/1pB1p2n/6Q1/2N1P2P/PPP3P1/5RK1 w - - 2 25");
    //printBoard(b); // View visual board for test

    // Black pawn on g7
    Colour colour = Colour::BLACK;
    uint8_t square = algebraicToSquare("g7");
    std::vector<Move> legalMoves;
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    std::vector<Move> expectedLegalMoves = {Move(square, square - 16), 
                                            Move(square, square - 8)};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // White pawn on g2
    colour = Colour::WHITE;
    square = algebraicToSquare("g2");
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {Move(square, square + 8)};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();

    // Black pawn on h7
    colour = Colour::BLACK;
    square = algebraicToSquare("h7");
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();


    b.movePiece(algebraicToSquare("f1"), algebraicToSquare("f8"));
    //printBoard(b); // View visual board for test

    // Black pawn on c7
    colour = Colour::BLACK;
    square = algebraicToSquare("c7");
    MoveGenerator::legalMoves(b, Piece::PAWN, colour, square, legalMoves);
    expectedLegalMoves = {};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
    legalMoves.clear();
}