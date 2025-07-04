#include <cstdint>
#include <optional>
#include <array>
#include <utility>
#include <cassert>
#include <bit>
#include "board/board.h"
#include "move/move.h"
#include "chess_types.h"

using Chess::toIndex;
using Chess::fromIndex;
using Bitboard = Board::Bitboard;
using Piece = Board::Piece;
using Colour = Board::Colour;

Board::Board() : castlingRights{{{true, true}, {true, true}}},
                 enPassantSquare(std::nullopt) {

    resetPieces();
}

std::optional<Colour> Board::getColour(uint8_t square) const {
    assert(square < 64 && "square must be between 0-63");
    uint64_t mask = 1ULL << square;
    if (whitePiecesBitboard & mask) {
        return std::optional<Colour>(Colour::WHITE);
    } else if (blackPiecesBitboard & mask) {
        return std::optional<Colour>(Colour::BLACK);
    }
    return std::nullopt;
}

std::pair<std::optional<Piece>, std::optional<Colour>> Board::getPieceAndColour(uint8_t square) const {
    assert(square < 64 && "square must be between 0-63");
    uint64_t mask = 1ULL << square;
    for (uint8_t i = 0; i < 6; i++) {
        for (uint8_t j = 0; j < 2; j++) {
            if (getBitboard(fromIndex<Piece>(i), fromIndex<Colour>(j)) & mask) {
                return {std::optional<Piece>(fromIndex<Piece>(i)), 
                        std::optional<Colour>(fromIndex<Colour>(j))};
            }
        }
    }

    return {};
}

std::optional<Piece> Board::getPiece(uint8_t square) const {
    return getPieceAndColour(square).first;
}

void Board::addPiece(Piece piece, Colour colour, uint8_t square) {
    assert(square < 64 && "square must be between 0-63");
    Bitboard& board = (colour == Colour::WHITE) ? whitePiecesBitboard : blackPiecesBitboard;
    uint64_t mask = 1ULL << square;
    board |= mask;
    pieceBitboards[toIndex(colour)][toIndex(piece)] |= mask;
    piecesBitboard |= mask;
}

void Board::removePiece(Piece piece, Colour colour, uint8_t square) {
    assert(square < 64 && "square must be between 0-63");
    Bitboard& board = (colour == Colour::WHITE) ? whitePiecesBitboard : blackPiecesBitboard;
    uint64_t mask = ~(1ULL << square);
    board &= mask;
    pieceBitboards[toIndex(colour)][toIndex(piece)] &= mask;
    piecesBitboard &= mask;
}

void Board::removePiece(uint8_t square) {
    auto [piece, colour] = getPieceAndColour(square);
    assert(piece.has_value() && "No piece seems to occupy fromSquare");
    assert(colour.has_value() && "No colour seems to occupy fromSquare");
    removePiece(*piece, *colour, square);
}

void Board::movePiece(Piece piece, Colour colour, uint8_t fromSquare, uint8_t toSquare) {
    removePiece(piece, colour, fromSquare);
    addPiece(piece, colour, toSquare);
}

void Board::movePiece(uint8_t fromSquare, uint8_t toSquare) {
    auto [piece, colour] = getPieceAndColour(fromSquare);
    assert(piece.has_value() && "No piece seems to occupy fromSquare");
    assert(colour.has_value() && "No colour seems to occupy fromSquare");
    movePiece(*piece, *colour, fromSquare, toSquare);
}

void Board::undo(Move& move, Colour oldPlayerTurn, std::array<std::array<bool, 2>, 2> oldCastlingRights, 
                                                                std::optional<uint8_t> oldEnPassantSquare) {

    uint8_t fromSquare = move.getFromSquare();
    uint8_t toSquare = move.getToSquare();

    // Remove promoted piece if it exists
    uint8_t promotion = move.getPromotionPiece();
    if (promotion != Move::NO_PROMOTION) {
        removePiece(fromIndex<Piece>(promotion), oldPlayerTurn, toSquare);
        addPiece(Piece::PAWN, oldPlayerTurn, fromSquare);
    } else {
        Piece movedPiece = *getPiece(toSquare);

        // Move piece back
        movePiece(movedPiece, oldPlayerTurn, toSquare, fromSquare);
    }

    // Place captured piece back if it exists
    uint8_t capture = move.getCapturedPiece();
    if (capture != Move::NO_CAPTURE) {
        Piece capturedPiece = fromIndex<Piece>(capture);
        Colour capturedColour = (oldPlayerTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
        uint8_t capturedSquare = (move.getEnPassant() != Move::NO_EN_PASSANT) ? 
                                 *oldEnPassantSquare :
                                 toSquare;

        addPiece(capturedPiece, capturedColour, capturedSquare);
    }

    // Place rook back if castled
    uint8_t castle = move.getCastling();
    if (castle != Move::NO_CASTLE) {
        constexpr uint8_t beforeCastleRookSquares[2][2] = {{7, 0}, {63, 56}}; // Indexed [colour][kingside/queenside]
        constexpr uint8_t afterCastleRookSquares[2][2] = {{5, 3}, {61, 59}}; // Indexed [colour][kingside/queenside]
        uint8_t castlingType = std::countr_zero(castle);

        uint8_t beforeRookSquare = beforeCastleRookSquares[toIndex(oldPlayerTurn)][castlingType];
        uint8_t afterRookSquare = afterCastleRookSquares[toIndex(oldPlayerTurn)][castlingType];

        movePiece(Piece::ROOK, oldPlayerTurn, afterRookSquare, beforeRookSquare);
    }

    castlingRights = oldCastlingRights; // Restore castling rights
    enPassantSquare = oldEnPassantSquare; // Restore en passant square
}

void Board::resetBoard() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            castlingRights[i][j] = true;
        }
    }
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