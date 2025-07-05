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
     * @brief Makes a move updating the game state
     * @param fromSquare Square to move the piece from
     * @param toSquare Square to move the piece to
     * @return True if the move is legal, false otherwise
     */
    bool makeMove(uint8_t fromSquare, uint8_t toSquare);

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

    // /**
    //  * @brief Checks if a move is legal
    //  * @param piece Piece to move
    //  * @param colour Colour of piece
    //  * @param fromSquare Square to move the piece from
    //  * @param toSquare Square to move the piece to
    //  */
    // bool isLegalMove(Piece piece, Colour colour, uint8_t fromSquare, uint8_t toSquare);
};

#endif // GAME_H