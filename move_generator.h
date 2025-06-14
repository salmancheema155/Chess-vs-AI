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
 * Class which provides functions to obtain
 * legal moves
 */
class MoveGenerator {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;
    static std::vector<Move> legalMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);

private:
    static void legalPawnMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);
    static void legalKnightMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);
    static void legalBishopMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);
    static void legalRookMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);
    static void legalQueenMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);
    static void legalKingMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);
};

#endif // MOVE_H