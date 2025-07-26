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

namespace {
    constexpr uint8_t beforeCastleRookSquares[2][2] = {{7, 0}, {63, 56}}; // Indexed [colour][kingside/queenside]
    constexpr uint8_t afterCastleRookSquares[2][2] = {{5, 3}, {61, 59}}; // Indexed [colour][kingside/queenside]
}

Board::Board() : castlingRights{{{true, true}, {true, true}}},
                 enPassantSquare(std::nullopt) {

    resetPieces();
}

Colour Board::getColour(uint8_t square) const {
    assert(square < 64 && "square must be between 0-63");
    uint64_t mask = 1ULL << square;
    if (whitePiecesBitboard & mask) {
        return Colour::WHITE;
    } else if (blackPiecesBitboard & mask) {
        return Colour::BLACK;
    }
    return Colour::NONE;
}

std::pair<Piece, Colour> Board::getPieceAndColour(uint8_t square) const {
    assert(square < 64 && "square must be between 0-63");
    uint64_t mask = 1ULL << square;
    for (uint8_t i = 0; i < 6; i++) {
        for (uint8_t j = 0; j < 2; j++) {
            if (getBitboard(fromIndex<Piece>(i), fromIndex<Colour>(j)) & mask) {
                return {fromIndex<Piece>(i), 
                        fromIndex<Colour>(j)};
            }
        }
    }

    return {Piece::NONE, Colour::NONE};
}

Piece Board::getPiece(uint8_t square) const {
    return getPieceAndColour(square).first;
}

Piece Board::getPiece(Colour colour, uint8_t square) const {
    assert(square < 64 && "square must be between 0-63");
    uint64_t mask = 1ULL << square;
    for (uint8_t i = 0; i < 6; i++) {
        if (getBitboard(fromIndex<Piece>(i), colour) & mask) {
            return fromIndex<Piece>(i);
        }
    }

    return Piece::NONE;
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
    assert(piece != Piece::NONE && "No piece seems to occupy fromSquare");
    assert(colour != Colour::NONE && "No colour seems to occupy fromSquare");
    removePiece(piece, colour, square);
}

void Board::movePiece(Piece piece, Colour colour, uint8_t fromSquare, uint8_t toSquare) {
    removePiece(piece, colour, fromSquare);
    addPiece(piece, colour, toSquare);
}

void Board::movePiece(uint8_t fromSquare, uint8_t toSquare) {
    auto [piece, colour] = getPieceAndColour(fromSquare);
    assert(piece != Piece::NONE && "No piece seems to occupy fromSquare");
    assert(colour != Colour::NONE && "No colour seems to occupy fromSquare");
    movePiece(piece, colour, fromSquare, toSquare);
}

void Board::makeMove(const Move move, Colour playerTurn) {
    uint8_t fromSquare = move.getFromSquare();
    uint8_t toSquare = move.getToSquare();
    Piece piece = getPiece(fromSquare);

    // Remove castling rights if rook has moved
    std::array<std::array<uint8_t, 2>, 2> startingRookSquares = {{{7, 0}, {63, 56}}};
    if (castlingRights[toIndex(playerTurn)][toIndex(Castling::KINGSIDE)] && 
        fromSquare == startingRookSquares[toIndex(playerTurn)][toIndex(Castling::KINGSIDE)]) {
            nullifyCastlingRights(playerTurn, Castling::KINGSIDE);
            
    } else if (castlingRights[toIndex(playerTurn)][toIndex(Castling::QUEENSIDE)] && 
        fromSquare == startingRookSquares[toIndex(playerTurn)][toIndex(Castling::QUEENSIDE)]) {
            nullifyCastlingRights(playerTurn, Castling::QUEENSIDE);
    }

    // Remove castling rights if king has moved
    if (piece == Piece::KING) {
        nullifyCastlingRights(playerTurn, Castling::KINGSIDE);
        nullifyCastlingRights(playerTurn, Castling::QUEENSIDE);
    }

    // Remove captured piece if any
    uint8_t capture = move.getCapturedPiece();
    if (capture != Move::NO_CAPTURE) {
        Piece capturedPiece = fromIndex<Piece>(capture);
        Colour capturedColour = (playerTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
        uint8_t capturedSquare = (move.getEnPassant() != Move::NO_EN_PASSANT) ? 
                                *enPassantSquare :
                                toSquare;
        
        removePiece(capturedPiece, capturedColour, capturedSquare);

        // Remove castling rights if rook is captured
        if (capturedPiece == Piece::ROOK) {
            if (capturedSquare == beforeCastleRookSquares[toIndex(capturedColour)][toIndex(Castling::KINGSIDE)]) {
                nullifyCastlingRights(capturedColour, Castling::KINGSIDE);
            } else if (capturedSquare == beforeCastleRookSquares[toIndex(capturedColour)][toIndex(Castling::QUEENSIDE)]) {
                nullifyCastlingRights(capturedColour, Castling::QUEENSIDE);
            }
        }
    }

    // Move rook if castling
    uint8_t castle = move.getCastling();
    if (castle != Move::NO_CASTLE) {
        uint8_t beforeRookSquare = beforeCastleRookSquares[toIndex(playerTurn)][castle];
        uint8_t afterRookSquare = afterCastleRookSquares[toIndex(playerTurn)][castle];
        movePiece(Piece::ROOK, playerTurn, beforeRookSquare, afterRookSquare);

        // Remove castling rights after castling
        nullifyCastlingRights(playerTurn, Castling::KINGSIDE);
        nullifyCastlingRights(playerTurn, Castling::QUEENSIDE);
    }

    // Add promotion piece if promoting
    uint8_t promotion = move.getPromotionPiece();
    if (promotion != Move::NO_PROMOTION) {
        removePiece(Piece::PAWN, playerTurn, fromSquare); // Remove pawn
        addPiece(fromIndex<Piece>(promotion), playerTurn, toSquare); // Add promotion piece
    } else {
        movePiece(piece, playerTurn, fromSquare, toSquare); // Move piece if not promoting
    }

    // Update en passant square if pawn moves 2 forward
    uint8_t fromRank = getRank(fromSquare);
    uint8_t toRank = getRank(toSquare);
    if (piece == Piece::PAWN && (toRank == fromRank + 2 || fromRank == toRank + 2)) {
        enPassantSquare = std::optional<uint8_t>(toSquare);
    } else {
        enPassantSquare = std::nullopt;
    }
}

void Board::undo(const Move move, Colour oldPlayerTurn, std::array<std::array<bool, 2>, 2> oldCastlingRights, 
                                                                std::optional<uint8_t> oldEnPassantSquare) {

    uint8_t fromSquare = move.getFromSquare();
    uint8_t toSquare = move.getToSquare();

    // Remove promoted piece if it exists
    uint8_t promotion = move.getPromotionPiece();
    if (promotion != Move::NO_PROMOTION) {
        removePiece(fromIndex<Piece>(promotion), oldPlayerTurn, toSquare);
        addPiece(Piece::PAWN, oldPlayerTurn, fromSquare);
    } else {
        Piece movedPiece = getPiece(toSquare);

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
        uint8_t beforeRookSquare = beforeCastleRookSquares[toIndex(oldPlayerTurn)][castle];
        uint8_t afterRookSquare = afterCastleRookSquares[toIndex(oldPlayerTurn)][castle];

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

    constexpr std::array<std::array<Bitboard, 6>, 2> initialBitboards = 
        {{{0x000000000000FF00ULL, 0x0000000000000042ULL, 0x0000000000000024ULL, 
          0x0000000000000081ULL, 0x0000000000000008ULL, 0x0000000000000010ULL}, 
         {0x00FF000000000000ULL, 0x4200000000000000ULL, 0x2400000000000000ULL, 
          0x8100000000000000ULL, 0x0800000000000000ULL, 0x1000000000000000ULL}}};

    whitePiecesBitboard = 0ULL;
    for (int i = 0; i < 6; i++) {
        pieceBitboards[white][i] = initialBitboards[white][i];
        whitePiecesBitboard |= initialBitboards[white][i];
    }

    blackPiecesBitboard = 0ULL;
    for (int i = 0; i < 6; i++) {
        pieceBitboards[black][i] = initialBitboards[black][i];
        blackPiecesBitboard |= initialBitboards[black][i];
    }

    piecesBitboard = whitePiecesBitboard | blackPiecesBitboard;
}