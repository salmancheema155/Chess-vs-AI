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

TEST(enPassantMoveTest, board1) {
    Board b;

    b.setCustomBoardState("rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    //printBoard(b); // View visual board for test

    // White pawn on e5
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("e5");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("e6")),
                                            Move(square, algebraicToSquare("f6"), toIndex(Piece::KNIGHT)),
                                            Move(square, algebraicToSquare("d6"), toIndex(Piece::PAWN), 
                                                Move::NO_PROMOTION, Move::NO_CASTLE, 1)};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(enPassantMoveTest, board2) {
    Board b;

    b.setCustomBoardState("r1bqkbnr/p1p1p1pp/1pn5/3pPpP1/8/8/PPPP1P1P/RNBQKBNR w KQkq f6 0 4");
    //printBoard(b); // View visual board for test

    // White pawn on e5
    Colour colour = Colour::WHITE;
    uint8_t square = algebraicToSquare("e5");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("e6")),
                                            Move(square, algebraicToSquare("f6"), toIndex(Piece::PAWN), 
                                                Move::NO_PROMOTION, Move::NO_CASTLE, 1)};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // White pawn on g5
    colour = Colour::WHITE;
    square = algebraicToSquare("g5");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("g6")),
                            Move(square, algebraicToSquare("f6"), toIndex(Piece::PAWN), 
                                Move::NO_PROMOTION, Move::NO_CASTLE, 1)};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(enPassantMoveTest, board3) {
    Board b;

    b.setCustomBoardState("rnbqkbnr/1ppppppp/8/8/pP6/5NP1/P1PPPP1P/RNBQKB1R b KQkq b3 0 2");
    //printBoard(b); // View visual board for test

    // Black pawn on a4
    Colour colour = Colour::BLACK;
    uint8_t square = algebraicToSquare("a4");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("a3")),
                                            Move(square, algebraicToSquare("b3"), toIndex(Piece::PAWN), 
                                                Move::NO_PROMOTION, Move::NO_CASTLE, 1)};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}

TEST(enPassantMoveTest, board4) {
    Board b;

    b.setCustomBoardState("r1bqkb1r/p1ppp1p1/1pn2n2/8/P2P1pPp/1PN5/1BPQPP1P/2KR1BNR w kq g3 0 7");
    //printBoard(b); // View visual board for test

    // Black pawn on f4
    Colour colour = Colour::BLACK;
    uint8_t square = algebraicToSquare("f4");
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    std::vector<Move> expectedLegalMoves = {Move(square, algebraicToSquare("f3")),
                                            Move(square, algebraicToSquare("g3"), toIndex(Piece::PAWN), 
                                                Move::NO_PROMOTION, Move::NO_CASTLE, 1)};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";

    // Black pawn on h4
    colour = Colour::BLACK;
    square = algebraicToSquare("h4");
    legalMoves = MoveGenerator::legalMoves(b, Piece::PAWN, colour, square);
    expectedLegalMoves = {Move(square, algebraicToSquare("h3")),
                            Move(square, algebraicToSquare("g3"), toIndex(Piece::PAWN), 
                                Move::NO_PROMOTION, Move::NO_CASTLE, 1)};
    sortMoves(legalMoves);
    sortMoves(expectedLegalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}