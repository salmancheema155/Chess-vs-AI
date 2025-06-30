#include <gtest/gtest.h>
#include <cstdint>
#include <optional>
#include "move/move.h"
#include "move/move_generator.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;
using ChessMove::Move;
using ChessMove::makeMove;

TEST(MoveTest, checkMakeMove) {
    Move m = makeMove(Piece::PAWN, Colour::WHITE, 12, 28);
    EXPECT_EQ(m.piece, Piece::PAWN);
    EXPECT_EQ(m.colour, Colour::WHITE);
    EXPECT_EQ(m.fromSquare, 12);
    EXPECT_EQ(m.toSquare, 28);
    EXPECT_EQ(m.captureSquare, std::nullopt);

    m = makeMove(Piece::QUEEN, Colour::BLACK, 14, 45, std::optional<uint8_t>(45));
    EXPECT_EQ(m.piece, Piece::QUEEN);
    EXPECT_EQ(m.colour, Colour::BLACK);
    EXPECT_EQ(m.fromSquare, 14);
    EXPECT_EQ(m.toSquare, 45);
    EXPECT_EQ(m.captureSquare, 45);

    m = makeMove(Piece::BISHOP, Colour::BLACK, 7, 54, std::optional<uint8_t>(54));
    EXPECT_EQ(m.piece, Piece::BISHOP);
    EXPECT_EQ(m.colour, Colour::BLACK);
    EXPECT_EQ(m.fromSquare, 7);
    EXPECT_EQ(m.toSquare, 54);
    EXPECT_EQ(m.captureSquare, 54);

    m = makeMove(Piece::PAWN, Colour::BLACK, 12, 28, 37);
    EXPECT_EQ(m.piece, Piece::PAWN);
    EXPECT_EQ(m.colour, Colour::BLACK);
    EXPECT_EQ(m.fromSquare, 12);
    EXPECT_EQ(m.toSquare, 28);
    EXPECT_EQ(m.captureSquare, 37);
}