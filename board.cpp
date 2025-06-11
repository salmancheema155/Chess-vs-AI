#include "board.h"
#include "chess_types.h"

using Chess::toIndex;
using Bitboard = Board::Bitboard;
using Piece = Board::Piece;
using Colour = Board::Colour;

Board::Board() : currTurn(Colour::WHITE), 
                 kingsideCastle{true, true}, 
                 queensideCastle{true, true} {
    resetPieces();
}

Colour Board::getTurn() const {
    return currTurn;
}

Bitboard Board::getPiecesBitboard() const {
    return piecesBitboard;
}

Bitboard Board::getWhitePiecesBitboard() const {
    return whitePiecesBitboard;
}

Bitboard Board::getBlackPiecesBitboard() const {
    return blackPiecesBitboard;
}

bool Board::getKingsideCastle(Colour colour) const {
    return kingsideCastle[toIndex(colour)];
}

bool Board::getQueensideCastle(Colour colour) const {
    return queensideCastle[toIndex(colour)];
}

void Board::nullifyKingsideCastle(Colour colour) {
    kingsideCastle[toIndex(colour)] = false;
}

void Board::nullifyQueensideCastle(Colour colour) {
    queensideCastle[toIndex(colour)] = false;
}

void Board::switchTurn() {
    currTurn = (currTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
}

void Board::addPiece(Piece piece, Colour colour, uint8_t square) {
    Bitboard& board = (colour == Colour::WHITE) ? whitePiecesBitboard : blackPiecesBitboard;
    uint64_t mask = 1ULL << square;
    board |= mask;
    pieceBitboards[toIndex(colour)][toIndex(piece)] |= mask;
}

void Board::removePiece(Piece piece, Colour colour, uint8_t square) {
    Bitboard& board = (colour == Colour::WHITE) ? whitePiecesBitboard : blackPiecesBitboard;
    uint64_t mask = ~(1ULL << square);
    board &= mask;
    pieceBitboards[toIndex(colour)][toIndex(piece)] &= mask;
}

void Board::movePiece(Piece piece, Colour colour, uint8_t fromSquare, uint8_t toSquare) {
    removePiece(piece, colour, fromSquare);
    addPiece(piece, colour, toSquare);
}

void Board::resetPieces() {
    std::array<std::array<Bitboard, toIndex(Piece::COUNT)>, 2> initialBitboards = 
        {{{0x000000000000FF00ULL, 0x0000000000000042ULL, 0x0000000000000024ULL, 
          0x0000000000000081ULL, 0x0000000000000008ULL, 0x0000000000000010ULL}, 
         {0x00FF000000000000ULL, 0x4200000000000000ULL, 0x2400000000000000ULL, 
          0x8100000000000000ULL, 0x0800000000000000ULL, 0x1000000000000000ULL}}};
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < toIndex(Piece::COUNT); j++) {
            pieceBitboards[i][j] = initialBitboards[i][j];
        }
    }
}