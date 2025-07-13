#ifndef GAME_H
#define GAME_H

#include <stack>
#include <string>
#include <vector>
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
     * @param promotion Promotion flag indicating what piece is gained from promotion if any
     * @return True if the move is legal, false otherwise
     * @note Promotion flags are defined and documented in the Move class
     */
    bool makeMove(uint8_t fromSquare, uint8_t toSquare, uint8_t promotion);

    /**
     * @brief Reverts the game state to the previous position before the most recent move
     * @return True if there is a move to undo, false otherwise (board is at the starting position)
     */
    bool undo();

    /**
     * @brief Checks if the current player's turn piece occupies the square
     * @param square Square to check if a piece occupies the square
     * @return True if the current player's turn piece occupies the square, otherwise false
     */
    bool isCurrentPlayerOccupies(uint8_t square);

    /**
     * @brief Gets the legal moves for a piece on a square
     * @param square Square which the piece is located on
     * @return Vector of all legal moves for the piece
     * @note If a piece does not occupy the square, this function returns an empty vector
     */
    std::vector<Move> getLegalMoves(uint8_t square);

    /**
     * @brief Gets the type of move
     * @param fromSquare Square which the piece moves from
     * @param toSquare Square which the piece moves to
     * @return Type of move flag denoting which type of move is played
     * @note See game.cpp documentation for move flags
     */
    int getMoveType(uint8_t fromSquare, uint8_t toSquare);

private:
    GameState currentState;
    Board board;
    std::stack<GameState> gameStateHistory;
    std::stack<Move> moveHistory;
    Colour currentTurn;
};

#endif // GAME_H