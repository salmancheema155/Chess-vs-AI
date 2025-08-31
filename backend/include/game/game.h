#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <stack>
#include <string>
#include <vector>
#include <optional>
#include "board/board.h"
#include "game_state.h"
#include "move/move.h"
#include "move/move_info.h"
#include "check/check.h"
#include "chess_types.h"

enum class GameStateEvaluation : uint8_t {
    IN_PROGRESS = 0,
    CHECKMATE = 1,
    STALEMATE = 2,
    CHECK = 3,
    DRAW_BY_REPETITION = 4,
    DRAW_BY_INSUFFICIENT_MATERIAL = 5,
    DRAW_BY_FIFTY_MOVE_RULE = 6
};

class Game {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    Game();

    /**
     * @brief Gets the current player turn
     * @return The current player's turn
     */
    inline Colour getCurrentTurn() {
        return currentTurn;
    }

    /**
     * @brief Gets the current board state
     * @return Reference to board object representing current board state
     */
    inline Board& getBoard() {
        return board;
    }

    inline uint64_t getHash() {
        return gameStateHistory.top().hash;
    }

    /**
     * @brief Gets the colour that occupies a square
     * @param square Square to get the colour for
     * @return Colour that occupies the square
     */
    Colour getColour(uint8_t square);

    /**
     * @brief Gets the current game state evaluation for the opposite player of the player who just made their move
     * @return Current game state evaluation of the game
     * @note This function should be called after a move has been made to determine whether the opposing player
     * has been checkmated, stalemated, etc
     */
    GameStateEvaluation getCurrentGameStateEvaluation();

    /**
     * @brief Gets the current game state evaluation without checking for checkmate or stalemate
     * @return Current game state evaluation
     * @warning This function does not check for checkmates or stalemates and should be used cautiously
     */
    GameStateEvaluation quickGameStateEvaluation();

    /**
     * @brief Makes a move updating the game state
     * @param move Move to make
     * @note Promotion flags are defined and documented in the Move class
     * @attention This function does not check whether or not the move is legal
     */
    void makeMove(const Move move);

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
     * @warning There must be a corresponding makeMove call before calling this function
     */
    void undo();

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
     * @brief Gets information about a move
     * @param fromSquare Square which the piece moves from
     * @param toSquare Square which the piece moves to
     * @param promotion Promotion piece flag
     * @return Move object wrapped in std::optional containing move info if the move is legal, std::nullopt otherwise
     * @note See move_info.h for Moveinfo flags and move.h for more move flags
     */
    std::optional<MoveInfo> getMoveInfo(uint8_t fromSquare, uint8_t toSquare, uint8_t promotion);

    /**
     * @brief Checks if the move is a promotion move
     * @param fromSquare Square which the piece moves from
     * @param toSquare Square which the piece moves to
     * @return True if the move is a promotion move, false otherwise
     */
    bool isPromotionMove(uint8_t fromSquare, uint8_t toSquare);

    /**
     * @brief Makes a null move
     * @warning This function should only be used for null move pruning
     */
    void makeNullMove();

    /**
     * @brief Undos a null move
     * @warning This function should only be used for null move pruning and paired with a corresponding makeNullMove() call
     */
    void undoNullMove();

    // // TESTING PURPOSES ONLY
    // /**
    //  * @brief Sets the game state to a given state
    //  * @param fen FEN string representation of board state
    //  * @attention This function should only be used for testing and debugging
    //  */
    // void setCustomGameState(const char* fen);

private:
    Board board;
    std::stack<GameState> gameStateHistory;
    std::stack<Move> moveHistory;
    Colour currentTurn;

    /**
     * @brief Calculates if the game is a draw by the fifty move rule
     * @return True if the game is a draw, false otherwise
     */
    bool isDrawByFiftyMoveRule();

    /**
     * @brief Calculates if the game is a draw by three fold repetition
     * @return True if the game is a draw, false otherwise
     */
    bool isDrawByRepetition();

    /**
     * @brief Calculates if the game is a draw by insufficient material
     * @return True if the game is a draw, false otherwise
     */
    bool isDrawByInsufficientMaterial();

    /**
     * @brief Removes the most previous hash from the positionHistory
     */
    void undoHash();

    /**
     * @brief Checks if a move is an irreversible move
     * @param piece Piece that is moving
     * @param move Move to check if it is irreversible
     * @return True if the move is irreversible (pawn push, capture, castling or promotion)
     */
    bool isIrreversibleMove(Piece piece, Move move);

    inline static std::vector<Move> moveBuffer = [] {
        std::vector<Move> v;
        v.reserve(256);
        return v;
    }();

    inline static std::vector<uint64_t> positionHistory = [] {
        std::vector<uint64_t> v;
        v.reserve(400);
        return v;
    }();

    inline static std::vector<uint16_t> irreversiblePositionIndices = [] {
        std::vector<uint16_t> v;
        v.reserve(150);
        return v;
    }();
};

#endif // GAME_H