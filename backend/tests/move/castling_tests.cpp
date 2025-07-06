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
using Chess::Castling;
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

TEST(castlingMoveTest, board1) {
    Board b;

    b.setCustomBoardState("r3kb1r/p1ppqppp/bpn2n2/1B2p1B1/3PP3/2N2N2/PPP2PPP/R2QK2R w KQkq - 2 7");
    //printBoard(b); // View visual board for test

    // White king on e1
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("e1");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("d2")),
                                            Move(square, algebraicToSquare("e2")),
                                            Move(square, algebraicToSquare("f1")),
                                            Move(square, algebraicToSquare("g1"), Move::NO_CAPTURE, 
                                                Move::NO_PROMOTION, toIndex(Castling::KINGSIDE))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black king on e8
    colour = Colour::BLACK;
    square = algebraicToSquare("e8");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("d8")),
                            Move(square, algebraicToSquare("c8"), Move::NO_CAPTURE, 
                                Move::NO_PROMOTION, toIndex(Castling::QUEENSIDE))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(castlingMoveTest, board2) {
    Board b;

    b.setCustomBoardState("r3k2r/p1pp1ppp/bpnb1n2/1B2p1B1/1q1PP3/2N2N2/PPPQ1PPP/R3K2R w KQkq - 6 9");
    //printBoard(b); // View visual board for test

    // White king on e1
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("e1");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("d1")),
                                            Move(square, algebraicToSquare("e2")),
                                            Move(square, algebraicToSquare("f1")),
                                            Move(square, algebraicToSquare("g1"), Move::NO_CAPTURE, 
                                                Move::NO_PROMOTION, toIndex(Castling::KINGSIDE)),
                                            Move(square, algebraicToSquare("c1"), Move::NO_CAPTURE, 
                                                Move::NO_PROMOTION, toIndex(Castling::QUEENSIDE))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black king on e8
    colour = Colour::BLACK;
    square = algebraicToSquare("e8");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("d8")),
                            Move(square, algebraicToSquare("f8")),
                            Move(square, algebraicToSquare("e7")),
                            Move(square, algebraicToSquare("c8"), Move::NO_CAPTURE, 
                                Move::NO_PROMOTION, toIndex(Castling::QUEENSIDE)),
                            Move(square, algebraicToSquare("g8"), Move::NO_CAPTURE, 
                                Move::NO_PROMOTION, toIndex(Castling::KINGSIDE))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(castlingMoveTest, board3) {
    Board b;

    b.setCustomBoardState("r3k2r/p1p2ppp/1pnb1n2/1b2pQB1/1q1PP3/2N2N2/PPP2PPP/R3K2R w KQkq - 10 11");
    //printBoard(b); // View visual board for test

    // White king on e1
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("e1");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("d1")),
                                            Move(square, algebraicToSquare("d2")),
                                            Move(square, algebraicToSquare("c1"), Move::NO_CAPTURE, 
                                                Move::NO_PROMOTION, toIndex(Castling::QUEENSIDE))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black king on e8
    colour = Colour::BLACK;
    square = algebraicToSquare("e8");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("d8")),
                            Move(square, algebraicToSquare("e7")),
                            Move(square, algebraicToSquare("f8")),
                            Move(square, algebraicToSquare("g8"), Move::NO_CAPTURE, 
                                Move::NO_PROMOTION, toIndex(Castling::KINGSIDE))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(castlingMoveTest, board4) {
    Board b;

    b.setCustomBoardState("rn2kb1r/pp1p1ppp/2qb2n1/2p1p3/2PP4/2NB1N2/PP2PPPP/R2QKB1R b KQkq - 8 4");
    //printBoard(b); // View visual board for test

    // White king on e1
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("e1");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("d2"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black king on e8
    colour = Colour::BLACK;
    square = algebraicToSquare("e8");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("d8")),
                            Move(square, algebraicToSquare("e7"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(castlingMoveTest, board5) {
    Board b;

    b.setCustomBoardState("r3k2r/pp1p1ppp/n1q3n1/2pNpb2/1bPP3P/3B1NP1/PP2PP2/R2QKB1R w KQ - 3 11");
    //printBoard(b); // View visual board for test

    // White king on e1
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("e1");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    std::vector<Move> expectedLegalMoves = {};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black king on e8
    colour = Colour::BLACK;
    square = algebraicToSquare("e8");
    legalMoves = MoveGenerator::legalMoves(b, Piece::KING, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("d8")),
                            Move(square, algebraicToSquare("f8"))};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}