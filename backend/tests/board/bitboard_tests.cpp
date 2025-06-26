#include <gtest/gtest.h>
#include <optional>
#include <cstdint>
#include "board/board.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;

TEST(BitboardTest, checkBitboardGetters) {
    Board b;
    EXPECT_EQ(b.getBitboard(Colour::WHITE), 0x000000000000FFFF);
    EXPECT_EQ(b.getBitboard(Colour::BLACK), 0xFFFF000000000000);


    b.movePiece(12, 28); // e4
    EXPECT_EQ(b.getBitboard(Colour::WHITE), 
            0b00000000'00000000'00000000'00000000'00010000'00000000'11101111'11111111);

    EXPECT_EQ(b.getBitboard(Colour::BLACK), 0xFFFF000000000000);
    EXPECT_EQ(b.getBitboard(Piece::PAWN, Colour::WHITE), 
            0b00000000'00000000'00000000'00000000'00010000'00000000'11101111'00000000);
    
    EXPECT_EQ(b.getPiecesBitboard(), b.getWhitePiecesBitboard() | b.getBlackPiecesBitboard());


    b.movePiece(52, 36); // e5
    EXPECT_EQ(b.getBitboard(Colour::WHITE), 
            0b00000000'00000000'00000000'00000000'00010000'00000000'11101111'11111111);

    EXPECT_EQ(b.getBitboard(Colour::BLACK), 
            0b11111111'11101111'00000000'00010000'00000000'00000000'00000000'00000000);

    EXPECT_EQ(b.getBitboard(Piece::PAWN, Colour::BLACK), 
            0b00000000'11101111'00000000'00010000'00000000'00000000'00000000'00000000);

    EXPECT_EQ(b.getPiecesBitboard(), b.getWhitePiecesBitboard() | b.getBlackPiecesBitboard());


    b.movePiece(6, 21); // Nf3
    EXPECT_EQ(b.getBitboard(Colour::WHITE), 
            0b00000000'00000000'00000000'00000000'00010000'00100000'11101111'10111111);

    EXPECT_EQ(b.getBitboard(Colour::BLACK), 
            0b11111111'11101111'00000000'00010000'00000000'00000000'00000000'00000000);

    EXPECT_EQ(b.getBitboard(Piece::KNIGHT, Colour::WHITE), 
            0b00000000'00000000'00000000'00000000'00000000'00100000'00000000'00000010);

    EXPECT_EQ(b.getPiecesBitboard(), b.getWhitePiecesBitboard() | b.getBlackPiecesBitboard());


    b.movePiece(57, 42); // Nc6
    EXPECT_EQ(b.getBitboard(Colour::WHITE), 
            0b00000000'00000000'00000000'00000000'00010000'00100000'11101111'10111111);

    EXPECT_EQ(b.getBitboard(Colour::BLACK), 
            0b11111101'11101111'00000100'00010000'00000000'00000000'00000000'00000000);

    EXPECT_EQ(b.getBitboard(Piece::KNIGHT, Colour::BLACK), 
            0b01000000'00000000'00000100'00000000'00000000'00000000'00000000'00000000);

    EXPECT_EQ(b.getPiecesBitboard(), b.getWhitePiecesBitboard() | b.getBlackPiecesBitboard());


    b.movePiece(5, 26); // Bc4
    EXPECT_EQ(b.getBitboard(Colour::WHITE), 
            0b00000000'00000000'00000000'00000000'00010100'00100000'11101111'10011111);

    EXPECT_EQ(b.getBitboard(Colour::BLACK), 
            0b11111101'11101111'00000100'00010000'00000000'00000000'00000000'00000000);

    EXPECT_EQ(b.getBitboard(Piece::BISHOP, Colour::WHITE), 
            0b00000000'00000000'00000000'00000000'00000100'00000000'00000000'00000100);

    EXPECT_EQ(b.getPiecesBitboard(), b.getWhitePiecesBitboard() | b.getBlackPiecesBitboard());
}