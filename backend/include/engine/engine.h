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
     */
    static Move getMove(Game& game, int depth);

private:
    int minimax(Game& game, Board& board, int depth, int alpha, int beta, Chess::PieceColour colour);

    int evaluate(Game& game, Board& board);
};

#endif // ENGINE_H