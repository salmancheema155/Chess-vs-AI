#ifndef CHECK_H
#define CHECK_H

#include <cstdint>
#include "board.h"
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

    static GameState evaluateGameState(const Board& board, Colour colour);

private:
    static bool isInCheck(const Board& board, Colour colour);
    static bool hasMove(const Board& board, Colour colour);
};

#endif // CHECK_H