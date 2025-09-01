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
     * @param timeLimit Maximum time for search in ms
     * @param maxDepth Max depth for engine search
     * @param quiescenceDepth Max depth for quiescence search
     */
    Engine(int timeLimit, uint8_t maxDepth, uint8_t quiescenceDepth);

    /**
     * @brief Calculates the best move according to the engine
     * @param game Game object representing current game state
     * @return Best move according to the engine
     * @attention 
     * This function assumes that at least one legal move exists
     * Win/draw checks must be done before calling this function
     */
    Move getMove(Game& game);

    /**
     * @brief Get the maximum depth searched in the last getMove call
     * @return Maximum depth searched in the last getMove call
     */
    inline uint8_t getMaxDepthSearched() {
        return maxDepthSearched;
    }

    /**
     * @brief Gets the current evaluation of the game from white's perspective
     * @return Current evaluation of the game
     */
    inline int16_t getCurrentEvaluation() {
        return currentEvaluation;
    }

    /**
     * @brief Gets the previous move that the engine played
     * @return The previous move that the engine played
     * @attention This function must be called after at least one engine move
     */
    inline Move getPreviousMove() {
        return previousMove;
    }

private:
    /**
     * @brief Searches through game tree to find the best evaluation for a player assuming optimal moves from both sides
     * @param game Game object
     * @param depth Depth to search in game tree
     * @param alpha Minimax alpha variable for alpha-beta pruning
     * @param beta Minimax beta variable for alpha-beta pruning
     * @param state The current game state evaluation
     * @param isPVNode True if the parent call was a PV node, false otherwise
     * @param timeUp Function to check if current search time has exceeded
     * @param ply Number of half moves elapsed since the start of the search
     * @param extensionCount Number of extensions made
     * @param allowNullMove Allow null pruning
     * @return Evaluation of current game state at a specified depth
     */
    int16_t negamax(Game& game, int depth, int16_t alpha, int16_t beta, GameStateEvaluation state, bool isPVNode,
                    const std::function<bool()>& timeUp, uint8_t ply = 1, int extensionCount = 0, bool allowNullMove = true);

    /**
     * @brief Performs a quiescence search at leaf nodes of minimax
     * @param alpha Minimax alpha variable for alpha-beta pruning
     * @param beta Minimax beta variable for alpha-beta pruning
     * @param qdepth Maximum depth of quiescence search
     * @param state The current game state evaluation
     * @param ply Number of half moves elapsed since the start of the search
     * @return Evaluation of current game state taking into account quiescence search
     */
    int16_t quiescence(Game& game, int16_t alpha, int16_t beta, uint8_t qdepth, GameStateEvaluation state, uint8_t ply);

    TranspositionTable transpositionTable;
    TranspositionTable quiescenceTranspositionTable;

    const int TIME_LIMIT;
    const uint8_t MAX_DEPTH;
    const uint8_t QUIESCENCE_DEPTH;

    static constexpr int16_t DELTA_MARGIN = 150;
    static constexpr int MAX_EXTENSION_COUNT = 5;

    std::vector<Move> moveBuffer;
    std::vector<std::vector<Move>> negamaxMoveBuffers;
    std::vector<std::vector<Move>> quiescenceMoveBuffers;

    uint8_t maxDepthSearched = 0;
    int16_t currentEvaluation = 0.0;
    Move previousMove;
};

#endif // ENGINE_H