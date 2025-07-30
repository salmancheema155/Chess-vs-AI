#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <cstdint>
#include <unordered_map>
#include "engine/transposition_table.h"
#include "move/move.h"
#include "game/game.h"
#include "board/board.h"
#include "chess_types.h"

class Engine {
public:
    /**
     * Constructor
     * @param maxDepth Max depth for engine search
     * @param quiescenceDepth Max depth for quiescence search
     */
    Engine(uint8_t maxDepth, uint8_t quiescenceDepth);

    /**
     * @brief Calculates the best move according to the engine
     * @param game Game object representing current game state
     * @return Best move according to the engine
     * @attention 
     * This function assumes that at least one legal move exists
     * Win/draw checks must be done before calling this function
     */
    Move getMove(Game& game);

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
    int16_t negamax(Game& game, int depth, int16_t alpha, int16_t beta, const std::function<bool()>& timeUp);

    /**
     * @brief Performs a quiescence search at leaf nodes of minimax
     * @param alpha Minimax alpha variable for alpha-beta pruning
     * @param beta Minimax beta variable for alpha-beta pruning
     * @param qdepth Maximum depth of quiescence search
     * @param state The current game state evaluation
     * @return Evaluation of current game state taking into account quiescence search
     */
    int16_t quiescence(Game& game, int16_t alpha, int16_t beta, uint8_t qdepth, GameStateEvaluation state);

    TranspositionTable transpositionTable;
    TranspositionTable quiescenceTranspositionTable;

    const uint8_t MAX_DEPTH;
    const uint8_t QUIESCENCE_DEPTH;

    static constexpr uint8_t NULL_MOVE_REDUCTION = 2;
    static constexpr int16_t DELTA_MARGIN = 100;

    std::vector<Move> moveBuffer;
    std::vector<std::vector<Move>> negamaxMoveBuffers;
    std::vector<std::vector<Move>> quiescenceMoveBuffers;
};

#endif // ENGINE_H