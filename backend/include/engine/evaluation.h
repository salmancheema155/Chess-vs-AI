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
    /**
     * @brief Calculates the evaluation of the players pieces
     * @param board Board object representing current board state
     * @param colour Colour of player
     * @param phase Current game phase (0-1) with 1 = early game, 0 = end game
     * @return Evaluation of players pieces
     */
    static int16_t pieceValueEvaluation(Board& board, Colour colour, double phase);

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
     * @param ply Number of half moves elapsed since the start of the search
     * @return Evaluation of current game state (at depth 0)
     */
    static int16_t evaluate(Game& game, GameStateEvaluation state, uint8_t ply);

    /**
     * @brief Gets the value of a piece
     * @param piece Piece (0 = pawn, 1 = knight, 2 = bishop, 3 = rook, 4 = queen)
     * @return Value of piece
     */
    static inline int16_t getPieceValue(uint8_t piece) {
        return pieceEvals[piece];
    }

private:
    static int16_t orderingScore(const Move move, Board& board, const Move* bestMove = nullptr);

    static constexpr int16_t CHECKMATE_VALUE = 30000;

    static constexpr int16_t PAWN_VALUE = 100;
    static constexpr int16_t KNIGHT_VALUE = 320;
    static constexpr int16_t BISHOP_VALUE = 330;
    static constexpr int16_t ROOK_VALUE = 500;
    static constexpr int16_t QUEEN_VALUE = 900;
    static constexpr int16_t KING_VALUE = 10000;
    static constexpr int16_t pieceEvals[6] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, KING_VALUE};

    static constexpr int16_t BEST_MOVE_VALUE = 10000;

    static constexpr int16_t DOUBLED_PAWN_PENALTY = -12;
    static constexpr int16_t DOUBLED_PAWN_PENALTY_END_GAME = -17;
    static constexpr int16_t ISOLATED_PAWN_PENALTY = -8;
    static constexpr int16_t ISOLATED_PAWN_PENALTY_END_GAME = -13;
    static constexpr int16_t BACKWARD_PAWN_PENALTY = -10;
    static constexpr int16_t BACKWARD_PAWN_PENALTY_END_GAME = -15;

    static constexpr int16_t CONNECTED_PAWN_BONUS = 5;
    static constexpr int16_t CONNECTED_PAWN_BONUS_END_GAME = 10;
};

#endif // EVALUATION_H