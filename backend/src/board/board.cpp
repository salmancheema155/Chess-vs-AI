#include <cstdint>
#include <optional>
#include <array>
#include "board/board.h"
#include "chess_types.h"

using Chess::toIndex;
using Bitboard = Board::Bitboard;
using Piece = Board::Piece;
using Colour = Board::Colour;

Board::Board() : currTurn(Colour::WHITE), 
                 kingsideCastle{true, true}, 
                 queensideCastle{true, true},
                 enPassantSquare(std::nullopt) {
    resetPieces();
}

std::optional<Colour> Board::getColour(uint8_t square) const {
    if (whitePiecesBitboard & (1ULL << square)) {
        return std::optional<Colour>(Colour::WHITE);
    } else if (blackPiecesBitboard & (1ULL << square)) {
        return std::optional<Colour>(Colour::BLACK);
    }
    return std::nullopt;
}

void Board::addPiece(Piece piece, Colour colour, uint8_t square) {
    Bitboard& board = (colour == Colour::WHITE) ? whitePiecesBitboard : blackPiecesBitboard;
    uint64_t mask = 1ULL << square;
    board |= mask;
    pieceBitboards[toIndex(colour)][toIndex(piece)] |= mask;
    piecesBitboard |= mask;
}

void Board::removePiece(Piece piece, Colour colour, uint8_t square) {
    Bitboard& board = (colour == Colour::WHITE) ? whitePiecesBitboard : blackPiecesBitboard;
    uint64_t mask = ~(1ULL << square);
    board &= mask;
    pieceBitboards[toIndex(colour)][toIndex(piece)] &= mask;
    piecesBitboard &= mask;
}

void Board::movePiece(Piece piece, Colour colour, uint8_t fromSquare, uint8_t toSquare) {
    removePiece(piece, colour, fromSquare);
    addPiece(piece, colour, toSquare);
}

void Board::resetBoard() {
    currTurn = Colour::WHITE;
    kingsideCastle[0] = true, kingsideCastle[1] = true;
    queensideCastle[0] = true, queensideCastle[1] = true;
    enPassantSquare = std::nullopt;
    resetPieces();
}

void Board::resetPieces() {
    constexpr uint8_t white = toIndex(Colour::WHITE);
    constexpr uint8_t black = toIndex(Colour::BLACK);

    constexpr std::array<std::array<Bitboard, toIndex(Piece::COUNT)>, 2> initialBitboards = 
        {{{0x000000000000FF00ULL, 0x0000000000000042ULL, 0x0000000000000024ULL, 
          0x0000000000000081ULL, 0x0000000000000008ULL, 0x0000000000000010ULL}, 
         {0x00FF000000000000ULL, 0x4200000000000000ULL, 0x2400000000000000ULL, 
          0x8100000000000000ULL, 0x0800000000000000ULL, 0x1000000000000000ULL}}};

    whitePiecesBitboard = 0ULL;
    for (int i = 0; i < toIndex(Piece::COUNT); i++) {
        pieceBitboards[white][i] = initialBitboards[white][i];
        whitePiecesBitboard |= initialBitboards[white][i];
    }

    blackPiecesBitboard = 0ULL;
    for (int i = 0; i < toIndex(Piece::COUNT); i++) {
        pieceBitboards[black][i] = initialBitboards[black][i];
        blackPiecesBitboard |= initialBitboards[black][i];
    }

    piecesBitboard = whitePiecesBitboard | blackPiecesBitboard;
}