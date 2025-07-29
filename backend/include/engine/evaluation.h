#ifndef EVALUATION_H
#define EVALUATION_H

#include <cstdint>
#include "board/board.h"
#include "move/move.h"
#include "game/game.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;

class Evaluation {
public:
    static constexpr int16_t CHECKMATE_VALUE = 32000;

    /**
     * @brief Calculates the evaluation of the players pieces
     * @param board Board object representing current board state
     * @param colour Colour of player
     * @return Evaluation of players pieces
     */
    static int16_t pieceValueEvaluation(Board& board, Colour colour);

    /**
     * @brief Orders moves by predicted best to worse for normal negamax search
     * @param moves Moves vector to sort
     * @param board Board object representing current board state
     * @param bestMove Best move from previous depth if any
     */
    static void orderMoves(std::vector<Move>& moves, Board& board, const Move* bestMove = nullptr);

    /**
     * @brief Orders moves by predicted best to worse for quiescence search
     * @param moves Moves vector to sort
     * @param board Board object representing current board state
     * @param bestMove Best move from previous depth if any
     */
    static void orderQuiescenceMoves(std::vector<Move>& moves, Board& board);

    /**
     * @brief Evaluates the current game state
     * @param game Game object
     * @param state The current game state evaluation
     * @return Evaluation of current game state (at depth 0)
     */
    static int16_t evaluate(Game& game, GameStateEvaluation state);

private:
    static int16_t orderingScore(const Move move, Board& board, const Move* bestMove = nullptr);

    static constexpr int16_t PAWN_VALUE = 100;
    static constexpr int16_t KNIGHT_VALUE = 320;
    static constexpr int16_t BISHOP_VALUE = 330;
    static constexpr int16_t ROOK_VALUE = 500;
    static constexpr int16_t QUEEN_VALUE = 900;
    static constexpr int16_t KING_VALUE = 10000;
    static constexpr int16_t pieceEvals[6] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, KING_VALUE};

    static constexpr int16_t BEST_MOVE_VALUE = 20000;

    static constexpr int16_t DOUBLED_PAWN_PENALTY = -17;
    static constexpr int16_t DOUBLED_PAWN_PENALTY_END_GAME = -12;
    static constexpr int16_t ISOLATED_PAWN_PENALTY = -10;
    static constexpr int16_t ISOLATED_PAWN_PENALTY_END_GAME = -15;
};

#endif // EVALUATION_H