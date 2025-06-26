#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <vector>
#include <optional>
#include <cstdint>
#include "board/board.h"
#include "board/board_debug.h"
#include "tests/board/board_utilities.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;
using Chess::toIndex;
using Chess::fromIndex;

TEST(MovePiece, checkMovePiece) {
    Board b;
    b.movePiece(12, 28);
    EXPECT_EQ(b.getPieceAndColour(28).first, Piece::PAWN);
    EXPECT_EQ(b.getPieceAndColour(28).second, Colour::WHITE);
    ASSERT_FALSE(b.getPieceAndColour(12).first.has_value());
    ASSERT_FALSE(b.getPieceAndColour(12).second.has_value());

    b.movePiece(0, 22);
    EXPECT_EQ(b.getPiece(22), Piece::ROOK);
    EXPECT_EQ(b.getColour(22), Colour::WHITE);
    ASSERT_FALSE(b.getPiece(0).has_value());
    ASSERT_FALSE(b.getColour(0).has_value());

    b.movePiece(61, 37);
    EXPECT_EQ(b.getPiece(37), Piece::BISHOP);
    EXPECT_EQ(b.getColour(37), Colour::BLACK);
    ASSERT_FALSE(b.getPiece(61).has_value());
    ASSERT_FALSE(b.getColour(61).has_value());
}

TEST(MovePiece, checkAddAndRemovePiece) {
    Board b;
    b.addPiece(Piece::KNIGHT, Colour::BLACK, 34);
    EXPECT_EQ(b.getPiece(34), Piece::KNIGHT);
    EXPECT_EQ(b.getColour(34), Colour::BLACK);

    b.removePiece(34);
    ASSERT_FALSE(b.getPiece(34).has_value());
    ASSERT_FALSE(b.getColour(34).has_value());

    b.addPiece(Piece::QUEEN, Colour::BLACK, 27);
    EXPECT_EQ(b.getPiece(27), Piece::QUEEN);
    EXPECT_EQ(b.getColour(27), Colour::BLACK);

    b.addPiece(Piece::PAWN, Colour::WHITE, 34);
    EXPECT_EQ(b.getPiece(34), Piece::PAWN);
    EXPECT_EQ(b.getColour(34), Colour::WHITE);

    b.addPiece(Piece::ROOK, Colour::WHITE, 45);
    EXPECT_EQ(b.getPiece(45), Piece::ROOK);
    EXPECT_EQ(b.getColour(45), Colour::WHITE);

    b.removePiece(0);
    ASSERT_FALSE(b.getPiece(0).has_value());
    ASSERT_FALSE(b.getColour(0).has_value());

    b.removePiece(15);
    ASSERT_FALSE(b.getPiece(15).has_value());
    ASSERT_FALSE(b.getColour(15).has_value());

    b.removePiece(54);
    ASSERT_FALSE(b.getPiece(54).has_value());
    ASSERT_FALSE(b.getColour(54).has_value());

    b.removePiece(62);
    ASSERT_FALSE(b.getPiece(62).has_value());
    ASSERT_FALSE(b.getColour(62).has_value());
}