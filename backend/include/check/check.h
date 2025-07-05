#ifndef CHECK_H
#define CHECK_H

#include <cstdint>
#include "board/board.h"
#include "chess_types.h"

enum class GameState {
    IN_PROGRESS,
    CHECKMATE,
    STALEMATE,
    CHECK,
    DRAW_BY_REPETITION,
    DRAW_BY_INSUFFICIENT_MATERIAL,
    DRAW_BY_FIFTY_MOVE_RULE
};

class Check {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    static GameState evaluateGameState(Board& board, Colour colour);

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
    static bool hasMove(Board& board, Colour colour);
};

#endif // CHECK_H