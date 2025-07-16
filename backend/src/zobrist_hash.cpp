#include <cstdint>
#include <optional>
#include <utility>
#include <bit>
#include "zobrist_hash.h"
#include "zobrist_keys.h"
#include "board/board.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;
using Chess::toIndex;
using Chess::Castling;

namespace {
    static constexpr uint8_t castleRookSquares[2][2] = {{5, 3}, {61, 59}};
}

namespace Zobrist {
    
    uint64_t computeInitialHash(const Board& board, const Chess::PieceColour playerTurn) {
        uint64_t hash = 0;
        // Pieces hash
        for (uint8_t square = 0; square < 64; square++) {
            auto [piece, colour] = board.getPieceAndColour(square);
            
            if (piece.has_value() && colour.has_value()) {
                uint64_t key = zobristTable[toIndex(*colour)][toIndex(*piece)][square];
                hash ^= key;
            }
        }

        // Castling hash
        if (board.getCastlingRights(Colour::WHITE, Castling::KINGSIDE)) hash ^= zobristCastling[0];
        if (board.getCastlingRights(Colour::WHITE, Castling::QUEENSIDE)) hash ^= zobristCastling[1];
        if (board.getCastlingRights(Colour::BLACK, Castling::KINGSIDE)) hash ^= zobristCastling[2];
        if (board.getCastlingRights(Colour::BLACK, Castling::QUEENSIDE)) hash ^= zobristCastling[3];

        // En passant hash
        if (board.getEnPassantSquare().has_value()) hash ^= zobristEnPassant[Board::getFile(*(board.getEnPassantSquare()))];

        // Player turn hash
        if (playerTurn == Colour::BLACK) hash ^= zobristPlayerTurn;

        return hash;
    }

    uint64_t updateHash(uint64_t currentHash, const Move& move, const std::optional<uint8_t> oldEnPassantSquare,
                        const std::optional<uint8_t> newEnPassantSquare, 
                        const std::array<std::array<bool, 2>, 2> oldCastleRights, 
                        const std::array<std::array<bool, 2>, 2> newCastleRights, 
                        Chess::PieceColour playerTurn, Chess::PieceType movedPiece) {

        uint8_t fromSquare = move.getFromSquare();
        uint8_t toSquare = move.getToSquare();
        Colour captureColour = (playerTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;

        // Remove old piece hash
        currentHash ^= zobristTable[toIndex(playerTurn)][toIndex(movedPiece)][fromSquare];

        // Promotion move
        uint8_t promotion = move.getPromotionPiece();
        if (promotion != move.NO_PROMOTION) {
            // Add new promotion piece hash
            currentHash ^= zobristTable[toIndex(playerTurn)][promotion][toSquare];

        // No promotion move
        } else {
            // Add new piece hash (pawn in promotion disappears so this is not executed)
            currentHash ^= zobristTable[toIndex(playerTurn)][toIndex(movedPiece)][toSquare];
        }            

        // Deal with capture hash update
        uint8_t capturedPiece = move.getCapturedPiece();
        if (capturedPiece != Move::NO_CAPTURE) {

            // Capture square for en passant is different
            uint8_t capturedSquare = (move.getEnPassant() != Move::NO_EN_PASSANT) ? 
                                    *oldEnPassantSquare :
                                    toSquare;

            currentHash ^= zobristTable[toIndex(captureColour)][capturedPiece][capturedSquare];
        }

        // Deal with rook move in castling
        uint8_t castling = move.getCastling();
        if (castling != Move::NO_CASTLE) {
            uint8_t castleRookSquare = castleRookSquares[toIndex(playerTurn)][castling];
            
            currentHash ^= zobristTable[toIndex(playerTurn)][toIndex(Piece::ROOK)][castleRookSquare];
        }

        // Deal with update in castling rights
        for (uint8_t i = 0; i < 2; i++) {
            for (uint8_t j = 0; j < 2; j++) {
                if (oldCastleRights[i][j] != newCastleRights[i][j]) {
                    currentHash ^= zobristCastling[2 * i + j];
                }
            }
        }

        // Deal with update in en passant square
        if (oldEnPassantSquare != newEnPassantSquare) {
            if (oldEnPassantSquare.has_value()) {
                currentHash ^= zobristEnPassant[Board::getFile(*oldEnPassantSquare)];
            }
            if (newEnPassantSquare.has_value()) {
                currentHash ^= zobristEnPassant[Board::getFile(*newEnPassantSquare)];
            }
        }

        // Toggle player turn hash
        currentHash ^= zobristPlayerTurn;
        return currentHash;
    }
}