#include <gtest/gtest.h>
#include <cstdint>
#include <optional>
#include "move/move.h"
#include "move/move_generator.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;
using Chess::toIndex;

TEST(MoveTest, checkMoveConstructor) {
    Move m(12, 28);
    EXPECT_EQ(m.getFromSquare(), 12);
    EXPECT_EQ(m.getToSquare(), 28);
    EXPECT_EQ(m.getCapturedPiece(), Move::NO_CAPTURE);

    m = Move(14, 45, toIndex(Piece::QUEEN));
    EXPECT_EQ(m.getFromSquare(), 14);
    EXPECT_EQ(m.getToSquare(), 45);
    EXPECT_EQ(m.getCapturedPiece(), 4);

    m = Move(7, 54, toIndex(Piece::PAWN));
    EXPECT_EQ(m.getFromSquare(), 7);
    EXPECT_EQ(m.getToSquare(), 54);
    EXPECT_EQ(m.getCapturedPiece(), 0);

    m = Move(12, 28, toIndex(Piece::ROOK));
    EXPECT_EQ(m.getFromSquare(), 12);
    EXPECT_EQ(m.getToSquare(), 28);
    EXPECT_EQ(m.getCapturedPiece(), 3);
}