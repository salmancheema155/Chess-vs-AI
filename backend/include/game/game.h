#ifndef GAME_H
#define GAME_H

#include <stack>
#include <string>
#include "board/board.h"
#include "game_state.h"
#include "move/move.h"
#include "chess_types.h"

class Game {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    Game();

    /**
     * @brief Reverts the game state to the previous position before the most recent move
     * @return True if there is a move to undo, false otherwise (board is at the starting position)
     */
    bool undo();

private:
    GameState currentState;
    Board board;
    std::stack<GameState> gameStateHistory;
    std::stack<Move> moveHistory;
};

#endif // GAME_H