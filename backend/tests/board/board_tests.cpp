#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <vector>
#include <optional>
#include <cstdint>
#include "board/board.h"
#include "tests/board/board_debug.h"
#include "tests/board/board_utilities.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;
using Chess::toIndex;
using Chess::fromIndex;
using Chess::Castling;
using BoardUtils::getExpectedInitialStartingSquares;

void checkInitialBoard(Board& b) {
    for (uint8_t i = 0; i < 6; i++) {
        for (uint8_t j = 0; j < 2; j++) {
            Piece piece = fromIndex<Piece>(i);
            Colour colour = fromIndex<Colour>(j);

            auto squares = b.getSquares(piece, colour);
            std::sort(squares.begin(), squares.end());

            EXPECT_EQ(squares, getExpectedInitialStartingSquares(piece, colour));
        }
    }
}

TEST(BoardTest, InitialiseBoard) {
    Board b;

    checkInitialBoard(b);

    EXPECT_EQ(b.getCastlingRights(Colour::WHITE, Castling::KINGSIDE), true);
    EXPECT_EQ(b.getCastlingRights(Colour::BLACK, Castling::KINGSIDE), true);
    EXPECT_EQ(b.getCastlingRights(Colour::WHITE, Castling::QUEENSIDE), true);
    EXPECT_EQ(b.getCastlingRights(Colour::BLACK, Castling::QUEENSIDE), true);

    EXPECT_EQ(b.getEnPassantSquare(), std::nullopt);
}

TEST(BoardTest, ResetBoard) {
    Board b;
    b.movePiece(12, 28);
    b.movePiece(52, 38);
    b.resetBoard();
    checkInitialBoard(b);
}

TEST(BoardTest, CheckInitialKingPosition) {
    Board b;
    EXPECT_EQ(b.getKingSquare(Colour::WHITE), 4);
    EXPECT_EQ(b.getKingSquare(Colour::BLACK), 60);
}

TEST(BoardTest, CheckEmptySquares) {
    Board b;
    EXPECT_FALSE(b.isEmpty(3));
    EXPECT_FALSE(b.isEmpty(61));
    EXPECT_TRUE(b.isEmpty(25));
    EXPECT_TRUE(b.isEmpty(46));
    EXPECT_FALSE(b.isEmpty(63));
    EXPECT_TRUE(b.isEmpty(16));
    EXPECT_FALSE(b.isEmpty(0));
}

TEST(BoardTest, CheckOccupiedSquares) {
    Board b;
    EXPECT_TRUE(b.isOccupied(0));
    EXPECT_FALSE(b.isOccupied(16));
    EXPECT_TRUE(b.isOccupied(63));
    EXPECT_TRUE(b.isOccupied(56));
    EXPECT_FALSE(b.isOccupied(17));
    EXPECT_TRUE(b.isOccupied(48));
    EXPECT_FALSE(b.isOccupied(47));
}

TEST(BoardTest, CheckGetColour) {
    Board b;
    EXPECT_EQ(b.getColour(0), Colour::WHITE);

    EXPECT_EQ(b.getColour(16), Colour::NONE);

    EXPECT_EQ(b.getColour(34), Colour::NONE);

    EXPECT_EQ(b.getColour(48), Colour::BLACK);

    EXPECT_EQ(b.getColour(63), Colour::BLACK);
}

TEST(BoardTest, CheckGetPiece) {
    Board b;
    EXPECT_EQ(b.getPiece(0), Piece::ROOK);

    EXPECT_EQ(b.getPiece(16), Piece::NONE);

    EXPECT_EQ(b.getPiece(34), Piece::NONE);

    EXPECT_EQ(b.getPiece(48), Piece::PAWN);

    EXPECT_EQ(b.getPiece(61), Piece::BISHOP);
}

TEST(BoardTest, CheckGetEnPassantSquare) {
    Board b;
    ASSERT_FALSE(b.getEnPassantSquare());
}