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
     * @return Best move according to the engine
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
     * @return Evaluation of current game state at a specified depth
     */
    static int negamax(Game& game, int depth, int alpha, int beta);

    /**
     * @brief Evaluates the current game state
     * @param game Game object
     * @param state The current game state evaluation
     * @param depth Current depth remaining (typically 0)
     * @return Evaluation of current game state (at depth 0)
     */
    static int evaluate(Game& game, GameStateEvaluation state, int depth);

    /**
     * @brief Performs a quiescence search at leaf nodes of minimax
     * @param alpha Minimax alpha variable for alpha-beta pruning
     * @param beta Minimax beta variable for alpha-beta pruning
     * @param qdepth Maximum depth of quiescence search
     * @param state The current game state evaluation
     * @return Evaluation of current game state taking into account quiescence search
     */
    static int quiescence(Game& game, int alpha, int beta, int qdepth, GameStateEvaluation state);
};

#endif // ENGINE_H