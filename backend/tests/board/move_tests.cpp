#include <gtest/gtest.h>
#include <optional>
#include <cstdint>
#include "board/board.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;

TEST(MovePiece, checkMovePiece) {
    Board b;
    b.movePiece(12, 28);
    EXPECT_EQ(b.getPieceAndColour(28).first, Piece::PAWN);
    EXPECT_EQ(b.getPieceAndColour(28).second, Colour::WHITE);
    EXPECT_EQ(b.getPieceAndColour(12).first, Piece::NONE);
    EXPECT_EQ(b.getPieceAndColour(12).second, Colour::NONE);

    b.movePiece(0, 22);
    EXPECT_EQ(b.getPiece(22), Piece::ROOK);
    EXPECT_EQ(b.getColour(22), Colour::WHITE);
    EXPECT_EQ(b.getPiece(0), Piece::NONE);
    EXPECT_EQ(b.getColour(0), Colour::NONE);

    b.movePiece(61, 37);
    EXPECT_EQ(b.getPiece(37), Piece::BISHOP);
    EXPECT_EQ(b.getColour(37), Colour::BLACK);
    EXPECT_EQ(b.getPiece(61), Piece::NONE);
    EXPECT_EQ(b.getColour(61), Colour::NONE);
}

TEST(MovePiece, checkAddAndRemovePiece) {
    Board b;
    b.addPiece(Piece::KNIGHT, Colour::BLACK, 34);
    EXPECT_EQ(b.getPiece(34), Piece::KNIGHT);
    EXPECT_EQ(b.getColour(34), Colour::BLACK);

    b.removePiece(34);
    EXPECT_EQ(b.getPiece(34), Piece::NONE);
    EXPECT_EQ(b.getColour(34), Colour::NONE);

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
    EXPECT_EQ(b.getPiece(0), Piece::NONE);
    EXPECT_EQ(b.getColour(0), Colour::NONE);

    b.removePiece(15);
    EXPECT_EQ(b.getPiece(15), Piece::NONE);
    EXPECT_EQ(b.getColour(15), Colour::NONE);

    b.removePiece(54);
    EXPECT_EQ(b.getPiece(54), Piece::NONE);
    EXPECT_EQ(b.getColour(54), Colour::NONE);

    b.removePiece(62);
    EXPECT_EQ(b.getPiece(62), Piece::NONE);
    EXPECT_EQ(b.getColour(62), Colour::NONE);
}