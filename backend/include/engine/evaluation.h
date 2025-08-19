#ifndef EVALUATION_H
#define EVALUATION_H

#include <cstdint>
#include <cstring>
#include <utility>
#include "board/board.h"
#include "move/move.h"
#include "game/game.h"
#include "chess_types.h"

enum class MoveType : uint8_t {
    BEST = 0,
    PROMOTION = 1,
    CAPTURE = 2,
    KILLER = 3,
    HISTORY = 4
};

class Evaluation {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

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
     * @param ply Number of half moves elapsed since the start of the search
     * @param colour Colour of player making the moves
     * @param bestMove Best move from previous depth if any
     */
    static void orderMoves(std::vector<Move>& moves, Board& board, uint8_t ply, Colour colour, const Move* bestMove = nullptr);

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

    /**
     * @brief Clears the killer move table
     * @note This should be called after every search to prevent stale data
     */
    static void clearKillerMoveTable();

    /**
     * @brief Adds the given move to the killer move table
     * @param move Move to add to the table
     * @param ply Number of half moves elapsed since the start of the search
     */
    static void addKillerMove(Move move, uint8_t ply);

    /**
     * @brief Checks if a given move is a killer move
     * @param move Move to check if it is a killer move
     * @param ply Number of half move elapsed since the start of the search
     * @return True if the move is a killer move at ply, otherwise false
     */
    static bool isKillerMove(Move move, uint8_t ply);

    /**
     * @brief Adds the given move to the history heuristic table
     * @param move Move to add to the table
     * @param piece Piece that is moving
     * @param colour Colour of the player that made the move
     * @param depth Depth remaining of the search at the point of adding the history heuristic
     */
    static void addHistoryHeuristic(Move move, Piece piece, Colour colour, uint8_t depth);

    /**
     * @brief Ages each entry inside of the table to prevent stale entries
     * @note This function should be called after every search
     */
    static void ageHistoryHeuristicsTable();

    /**
     * @brief Clears the history heuristics table
     * @note This funcion should not often be called and instead values should be aged
     */
    static void clearHistoryHeuristicsTable();

private:
    static std::pair<MoveType, int16_t> orderingScore(const Move move, Board& board, uint8_t ply, Colour colour, const Move* bestMove = nullptr);
    static int16_t orderingQuiescenceScore(const Move move, Board& board);

    static constexpr int16_t CHECKMATE_VALUE = 30000;

    static constexpr int16_t PAWN_VALUE = 100;
    static constexpr int16_t KNIGHT_VALUE = 320;
    static constexpr int16_t BISHOP_VALUE = 330;
    static constexpr int16_t ROOK_VALUE = 500;
    static constexpr int16_t QUEEN_VALUE = 900;
    static constexpr int16_t KING_VALUE = 10000;
    static constexpr int16_t pieceEvals[6] = {PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, KING_VALUE};

    static constexpr int16_t PROMOTION_ORDERING_VALUE = 9000;

    static constexpr int16_t DOUBLED_PAWN_PENALTY = -12;
    static constexpr int16_t DOUBLED_PAWN_PENALTY_END_GAME = -17;
    static constexpr int16_t ISOLATED_PAWN_PENALTY = -8;
    static constexpr int16_t ISOLATED_PAWN_PENALTY_END_GAME = -13;
    static constexpr int16_t BACKWARD_PAWN_PENALTY = -10;
    static constexpr int16_t BACKWARD_PAWN_PENALTY_END_GAME = -15;

    static constexpr int16_t CONNECTED_PAWN_BONUS = 5;
    static constexpr int16_t CONNECTED_PAWN_BONUS_END_GAME = 10;

    static constexpr int16_t MAJOR_PAWN_SHIELD_BONUS = 15;
    static constexpr int16_t MINOR_PAWN_SHIELD_BONUS = 8;

    static constexpr int16_t MAX_TROPISM_DISTANCE = 4;
    static constexpr int16_t KING_TROPISM_QUEEN_BONUS = 20;
    static constexpr int16_t KING_TROPISM_ROOK_BONUS = 10;
    static constexpr int16_t KING_TROPISM_KNIGHT_BONUS = 12;
    static constexpr int16_t KING_TROPISM_BISHOP_BONUS = 5;
    static constexpr int16_t KING_TROPISM_BONUSES[4] = {KING_TROPISM_KNIGHT_BONUS, KING_TROPISM_BISHOP_BONUS, 
                                                          KING_TROPISM_ROOK_BONUS, KING_TROPISM_QUEEN_BONUS};

    static constexpr int16_t MAX_HISTORY_VALUE = 128;

    static Move killerMoves[256][2];
    static int16_t historyHeuristics[2][6][64][64];
};

#endif // EVALUATION_H