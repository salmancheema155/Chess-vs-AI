#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include <vector>
#include <cstdint>
#include <optional>
#include "chess_types.h"
#include "board.h"
#include "move.h"

/**
 * Class which provides functions to obtain legal moves given a current board position
 */
class MoveGenerator {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;
    using Move = ChessMove::Move;

    /**
     * @brief Gets the legal moves for a given piece
     * @param board Board object representing the current board state
     * @param piece Piece to find legal moves for
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     */
    static std::vector<Move> legalMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare);

private:
    /**
     * @brief Adds legal moves for a given piece to the given vector moves
     * @param board Board object representing the current board state
     * @param piece Piece to find legal moves for
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal moves to
     * @warning This function does not take into account moves where the king will be placed in a check
     * The vector moves will still be appended with moves where the king will be in direct danger
     * Use legalMoves function instead for exact list of legal moves
     */
    static void legalPawnMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::legalPawnMoves
    static void legalKnightMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::legalPawnMoves
    static void legalBishopMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::legalPawnMoves
    static void legalRookMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::legalPawnMoves
    static void legalQueenMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::legalPawnMoves
    static void legalKingMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);
};

#endif // MOVE_GENERATOR_H