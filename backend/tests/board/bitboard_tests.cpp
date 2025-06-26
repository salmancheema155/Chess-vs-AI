#include <gtest/gtest.h>
#include <optional>
#include <array>
#include <cstdint>
#include "board/board.h"
#include "chess_types.h"

using Bitboard = Chess::Bitboard;
using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;
using Chess::toIndex;
using Chess::fromIndex;

static const std::array<std::array<Bitboard, 6>, 2> initialPieceBitboards = {{
    {
        // WHITE
        0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'00000000, // PAWN
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'01000010, // KNIGHT
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00100100, // BISHOP
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'10000001, // ROOK
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00001000, // QUEEN
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00010000}, // KING
    {
        // BLACK
        0b00000000'11111111'00000000'00000000'00000000'00000000'00000000'00000000, // PAWN
        0b01000010'00000000'00000000'00000000'00000000'00000000'00000000'00000000, // KNIGHT
        0b00100100'00000000'00000000'00000000'00000000'00000000'00000000'00000000, // BISHOP
        0b10000001'00000000'00000000'00000000'00000000'00000000'00000000'00000000, // ROOK
        0b00001000'00000000'00000000'00000000'00000000'00000000'00000000'00000000, // QUEEN
        0b00010000'00000000'00000000'00000000'00000000'00000000'00000000'00000000 // KING
    }
    
}};

TEST(BitboardTest, checkInitialBitboards) {
    Board b;
    EXPECT_EQ(b.getBitboard(Colour::WHITE), 0x000000000000FFFF);
    EXPECT_EQ(b.getBitboard(Colour::BLACK), 0xFFFF000000000000);
    EXPECT_EQ(b.getPiecesBitboard(), b.getWhitePiecesBitboard() | b.getBlackPiecesBitboard());

    for (uint8_t i = 0; i < 2; i++) {
        for (uint8_t j = 0; j < toIndex(Piece::COUNT); j++) {
            EXPECT_EQ(b.getBitboard(fromIndex<Piece>(j), fromIndex<Colour>(i)), initialPieceBitboards[i][j]);
        }
    }
}

TEST(BitboardTest, checkBitboardGetters) {
    Board b;

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