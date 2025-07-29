#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include <vector>
#include <cstdint>
#include <optional>
#include "chess_types.h"
#include "board/board.h"
#include "move/move.h"

/**
 * Class which provides functions to obtain legal moves given a current board position
 */
class MoveGenerator {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    /**
     * @brief Gets the legal moves for a given piece and colour
     * @param board Board object representing the current board state
     * @param piece Piece to find legal moves for
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Legal moves out paramater
     */
    static void legalMoves(Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /**
     * @brief Gets the legal moves for a given colour
     * @param board Board object representing the current board state
     * @param colour Colour of player
     * @param moves Legal moves out paramater
     */
    static void legalMoves(Board& board, Colour colour, std::vector<Move>& moves);

    /**
     * @brief Gets the legal captures for a given piece and colour
     * @param board Board object representing the current board state
     * @param piece Piece to find legal moves for
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Capture moves out paramater
     */
    static void legalCaptures(Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /**
     * @brief Gets the legal captures for a given colour
     * @param board Board object representing the current board state
     * @param colour Colour of player
     * @param moves Capture moves out paramater
     */
    static void legalCaptures(Board& board, Colour colour, std::vector<Move>& moves);

    /**
     * @brief Gets the legal moves which land a player in check or are capture moves or is a queen promotion for a given piece and colour
     * @param board Board object representing the current board state
     * @param piece Piece to find legal moves for
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Moves out paramater
     */
    static void quiescenceMoves(Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /**
     * @brief Gets the legal moves which land a player in check or are capture moves or is a queen promotion for a given colour
     * @param board Board object representing the current board state
     * @param colour Colour of player
     * @param moves Moves out paramater
     */
    static void quiescenceMoves(Board& board, Colour colour, std::vector<Move>& moves);

private:
    /**
     * @brief Filters out illegal moves
     * @param board Board object representing current board state
     * @param colour Colour of player's moves
     * @param moves Moves out paramater
     */
    static void filterIllegalMoves(Board& board, Colour colour, std::vector<Move>& moves);

    /**
     * @brief Filters out illegal moves and moves which are neither checks nor captures nor a queen promotion
     * @param board Board object representing current board state
     * @param colour Colour of player's moves
     * @param moves Moves out paramater
     */
    static void quiescenceFilter(Board& board, Colour colour, std::vector<Move>& moves);

    /**
     * @brief Adds pseudo legal moves to the given vector moves
     * @param board Board object representing the current board state
     * @param piece Type of piece
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal moves to
     * @warning This function does not take into account moves where the king will be placed in a check
     * The vector moves may still append with moves where the king will be in direct danger
     * Use legalMoves function instead for exact list of legal moves
     */
    static void pseudoLegalMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /**
     * @brief Adds pseudo legal capture moves to the given vector moves
     * @param board Board object representing the current board state
     * @param piece Type of piece
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal moves to
     * @warning This function does not take into account moves where the king will be placed in a check
     * The vector moves may still append with moves where the king will be in direct danger
     * Use legalCaptures function instead for exact list of legal moves
     */
    static void pseudoLegalCaptures(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /**
     * @brief Adds pseudo legal moves to the given vector moves
     * @param board Board object representing the current board state
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal moves to
     * @warning This function does not take into account moves where the king will be placed in a check
     * The vector moves may still append with moves where the king will be in direct danger
     * Use legalMoves function instead for exact list of legal moves
     */
    static void pseudoLegalPawnMoves(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnMoves
    static void pseudoLegalKnightMoves(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnMoves
    static void pseudoLegalBishopMoves(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnMoves
    static void pseudoLegalRookMoves(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnMoves
    static void pseudoLegalQueenMoves(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnMoves
    static void pseudoLegalKingMoves(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /**
     * @brief Adds pseudo legal capture moves to the given vector moves
     * @param board Board object representing the current board state
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal moves to
     * @warning This function does not take into account moves where the king will be placed in a check
     * The vector moves may still append with moves where the king will be in direct danger
     */
    static void pseudoLegalPawnCaptures(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnCaptures
    static void pseudoLegalKnightCaptures(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnCaptures
    static void pseudoLegalBishopCaptures(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnCaptures
    static void pseudoLegalRookCaptures(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnCaptures
    static void pseudoLegalQueenCaptures(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);

    /// \copydoc MoveGenerator::pseudoLegalPawnCaptures
    static void pseudoLegalKingCaptures(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves);
};

#endif // MOVE_GENERATOR_H