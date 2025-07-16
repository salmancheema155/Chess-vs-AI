#ifndef CHECK_H
#define CHECK_H

#include <cstdint>
#include "board/board.h"
#include "chess_types.h"

enum class CheckEvaluation {
    CHECKMATE,
    STALEMATE,
    CHECK,
    NONE
};

class Check {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    /**
     * @brief Evaluates the current game state for a player
     * @param board Board object representing the current board state
     * @param colour Colour of the player to check the current game state for
     * @return CheckEvaluation enum values representing the evaluation of the current game state for the specified player
     */
    static CheckEvaluation evaluateGameState(Board& board, Colour colour);

    /**
     * @brief Checks if there is a check on the specified coloured king
     * @param board Board object representing current board state
     * @param colour Colour of king
     * @return True if in check, false otherwise
     */
    static bool isInCheck(const Board& board, Colour colour);

    /**
     * @brief Checks if a square is in danger
     * @param board Board object representing current board state
     * @param colour Colour of piece occupying the square (opposite colour is the colour of pieces attacking that square)
     * @param targetSquare Square to check if it is in danger
     * @note There is no requirement that a piece occupies the square
     */
    static bool isInDanger(const Board& board, Colour colour, uint8_t targetSquare);

private:
    /**
     * @brief Checks if a player has a legal move
     * @param board Board object representing current board state
     * @param colour Colour of player
     * @return True if player has a legal move, false otherwise
     */
    static bool hasMove(Board& board, Colour colour);
};

#endif // CHECK_H