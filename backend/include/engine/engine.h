#ifndef ENGINE_H
#define ENGINE_H

#include "move/move.h"
#include "game/game.h"
#include "board/board.h"
#include "chess_types.h"

class Engine {
public:
    Engine();

    /**
     * @brief Calculates the best move according to the engine
     * @param game Game object representing current game state
     * @param depth Depth to search in game tree
     * @attention 
     * This function assumes that at least one legal move exists
     * If no legal move exists, then the function returns an empty move
     * Win/draw checks should be done before calling this function
     */
    static Move getMove(Game& game, int depth);

private:
    /**
     * @brief Searches through game tree to find the best evaluation for a player assuming optimal moves from both sides
     * @param game Game object
     * @param depth Depth to search in game tree
     * @param alpha Minimax alpha variable for alpha-beta pruning
     * @param beta Minimax beta variable for alpha-beta pruning
     */
    static int minimax(Game& game, int depth, int alpha, int beta);

    /**
     * @brief Evaluates the current game state
     * @param game Game object
     * @param state The current game state evaluation
     */
    static int evaluate(Game& game, GameStateEvaluation& state);
};

#endif // ENGINE_H