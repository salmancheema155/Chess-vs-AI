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

namespace {
    static constexpr uint8_t castleRookSquares[4] = {5, 3, 61, 59};
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
        if (board.getKingsideCastle(Colour::WHITE)) hash ^= zobristCastling[0];
        if (board.getQueensideCastle(Colour::WHITE)) hash ^= zobristCastling[1];
        if (board.getKingsideCastle(Colour::BLACK)) hash ^= zobristCastling[2];
        if (board.getQueensideCastle(Colour::BLACK)) hash ^= zobristCastling[3];

        // En passant hash
        if (board.getEnPassantSquare().has_value()) hash ^= zobristEnPassant[Board::getFile(*(board.getEnPassantSquare()))];

        // Player turn hash
        if (playerTurn == Colour::BLACK) hash ^= zobristPlayerTurn;

        return hash;
    }

    uint64_t updateHash(uint64_t currentHash, const Move& move, const Board& oldBoard, const Board& newBoard) {
        uint8_t fromSquare = move.getFromSquare();
        uint8_t toSquare = move.getToSquare();

        // Promotion move
        uint8_t promotion = move.getPromotionPiece();
        if (promotion != Move::NO_PROMOTION) {
            Colour colour = *(oldBoard.getColour(fromSquare));

            currentHash ^= zobristTable[toIndex(colour)][toIndex(Piece::PAWN)][fromSquare];
            currentHash ^= zobristTable[toIndex(colour)][promotion][toSquare];
            
        // No promotion move
        } else {
            auto [piece, colour] = newBoard.getPieceAndColour(toSquare);

            // Remove old piece hash and update with new piece hash
            currentHash ^= zobristTable[toIndex(*colour)][toIndex(*piece)][fromSquare];
            currentHash ^= zobristTable[toIndex(*colour)][toIndex(*piece)][toSquare];

            // Deal with capture hash update
            uint8_t capturedPiece = move.getCapturedPiece();
            if (capturedPiece != Move::NO_CAPTURE) {

                // Capture square for en passant is different
                uint8_t capturedSquare = (move.getEnPassant() != Move::NO_EN_PASSANT) ? 
                                        *(oldBoard.getEnPassantSquare()) :
                                        toSquare;

                Colour capturedColour = *(oldBoard.getColour(capturedSquare));
                currentHash ^= zobristTable[toIndex(capturedColour)][capturedPiece][capturedSquare];
            }

            // Deal with castling hash update
            uint8_t castling = move.getCastling();
            if (castling != Move::NO_CASTLE) {
                uint8_t setBitIndex = std::countr_zero(castling);
                uint8_t castleRookSquare = castleRookSquares[setBitIndex];
                
                currentHash ^= zobristTable[toIndex(*colour)][toIndex(Piece::ROOK)][castleRookSquare];
                currentHash ^= zobristCastling[setBitIndex];
            }

            // Deal with en passant hash update
            std::optional<uint8_t> oldEnPassantSquare = oldBoard.getEnPassantSquare();
            std::optional<uint8_t> newEnPassantSquare = newBoard.getEnPassantSquare();
            if (oldEnPassantSquare != newEnPassantSquare) {
                if (oldEnPassantSquare.has_value()) {
                    currentHash ^= zobristEnPassant[Board::getFile(*oldEnPassantSquare)];
                }
                if (newEnPassantSquare.has_value()) {
                    currentHash ^= zobristEnPassant[Board::getFile(*newEnPassantSquare)];
                }
            }
        }

        // Toggle player turn hash
        currentHash ^= zobristPlayerTurn;
        return currentHash;
    }
}