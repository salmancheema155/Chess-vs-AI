#ifndef MOVE_H
#define MOVE_H

#include <vector>
#include <cstdint>
#include <optional>
#include "chess_types.h"
#include "board.h"

/**
 * Struct holding data to represent a move
 */
struct Move {
    Chess::PieceType piece;
    Chess::PieceColour colour;
    uint8_t fromSquare;
    uint8_t toSquare;
    std::optional<uint8_t> captureSquare = std::nullopt;
};

/**
 * Class which provides provides functions to obtain
 * legal moves
 */
class MoveGenerator {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;
    static std::vector<Move> legalMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);

private:
    static std::vector<Move> legalPawnMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);
    static std::vector<Move> legalKnightMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);
    static std::vector<Move> legalBishopMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);
    static std::vector<Move> legalRookMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);
    static std::vector<Move> legalQueenMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);
    static std::vector<Move> legalKingMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);

    static inline Move makeMove(Chess::PieceType piece, Chess::PieceColour colour, 
                                uint8_t fromSquare, uint8_t toSquare, 
                                std::optional<uint8_t> captureSquare = std::nullopt);
};

#endif // MOVE_H