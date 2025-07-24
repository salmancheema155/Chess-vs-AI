#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>
#include <unordered_map>
#include "engine/transposition_table.h"
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
     * @return Best move according to the engine
     * @attention 
     * This function assumes that at least one legal move exists
     * Win/draw checks must be done before calling this function
     */
    static Move getMove(Game& game);

private:
    /**
     * @brief Searches through game tree to find the best evaluation for a player assuming optimal moves from both sides
     * @param game Game object
     * @param depth Depth to search in game tree
     * @param alpha Minimax alpha variable for alpha-beta pruning
     * @param beta Minimax beta variable for alpha-beta pruning
     * @param timeUp Function to check if current search time has exceeded
     * @return Evaluation of current game state at a specified depth
     */
    static int16_t negamax(Game& game, uint8_t depth, int16_t alpha, int16_t beta, const std::function<bool()>& timeUp);

    /**
     * @brief Evaluates the current game state
     * @param game Game object
     * @param state The current game state evaluation
     * @param depth Current depth remaining (typically 0)
     * @return Evaluation of current game state (at depth 0)
     */
    static int16_t evaluate(Game& game, GameStateEvaluation state, uint8_t depth);

    /**
     * @brief Performs a quiescence search at leaf nodes of minimax
     * @param alpha Minimax alpha variable for alpha-beta pruning
     * @param beta Minimax beta variable for alpha-beta pruning
     * @param qdepth Maximum depth of quiescence search
     * @param state The current game state evaluation
     * @return Evaluation of current game state taking into account quiescence search
     */
    static int16_t quiescence(Game& game, int16_t alpha, int16_t beta, uint8_t qdepth, GameStateEvaluation state);

    static TranspositionTable transpositionTable;
    static TranspositionTable quiescenceTranspositionTable;
};

#endif // ENGINE_H