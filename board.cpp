#include "board.h"
#include "chess_types.h"

Board::Board() : currTurn(Chess::PieceColour::WHITE), 
                 kingsideCastle{true, true}, 
                 queensideCastle{true, true} {
    pieceBitboards[0][0] = 0x000000000000FF00ULL;
    pieceBitboards[0][1] = 0x0000000000000042ULL;
    pieceBitboards[0][2] = 0x0000000000000024ULL;
    pieceBitboards[0][3] = 0x0000000000000081ULL;
    pieceBitboards[0][4] = 0x0000000000000008ULL;
    pieceBitboards[0][5] = 0x0000000000000010ULL;

    pieceBitboards[1][0] = 0x00FF000000000000ULL;
    pieceBitboards[1][1] = 0x4200000000000000ULL;
    pieceBitboards[1][2] = 0x2400000000000000ULL;
    pieceBitboards[1][3] = 0x8100000000000000ULL;
    pieceBitboards[1][4] = 0x0800000000000000ULL;
    pieceBitboards[1][5] = 0x1000000000000000ULL;
}

Chess::PieceColour Board::getTurn() {
    return currTurn;
}

bool Board::getKingsideCastle(Chess::PieceColour colour) {
    return kingsideCastle[Chess::toIndex(colour)];
}

bool Board::getQueensideCastle(Chess::PieceColour colour) {
    return queensideCastle[Chess::toIndex(colour)];
}

void Board::nullifyKingsideCastle(Chess::PieceColour colour) {
    kingsideCastle[Chess::toIndex(colour)] = false;
}

void Board::nullifyQueensideCastle(Chess::PieceColour colour) {
    queensideCastle[Chess::toIndex(colour)] = false;
}

void Board::switchTurn() {
    currTurn = (currTurn == Chess::PieceColour::WHITE) 
              ? Chess::PieceColour::BLACK 
              : Chess::PieceColour::WHITE;
}

void Board::addPiece(Chess::PieceType piece, Chess::PieceColour colour, uint8_t square) {
    pieceBitboards[Chess::toIndex(colour)][Chess::toIndex(piece)] |= (1ULL << square);
}

void Board::removePiece(Chess::PieceType piece, Chess::PieceColour colour, uint8_t square) {
    pieceBitboards[Chess::toIndex(colour)][Chess::toIndex(piece)] &= ~((1ULL) << square);
}