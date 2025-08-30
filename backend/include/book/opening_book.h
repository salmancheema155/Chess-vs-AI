#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <random>
#include "board/board.h"
#include "move/move.h"

class OpeningBook {
public:
    /**
     * @brief Loads the book with appropriate opening entries
     * @attention This function must be called once before using the book
     */
    static void loadBook();

    /**
     * @brief Gets a random book move
     * @param hash Zobrist hash representing current game state
     * @return Random book move if there exists a move in the book in the given position, otherwise a null move
     * @warning The move that this function returns does not take into account captures, en passant or castling - use the overload to take this into account
     * @attention The move returned only represents the from square, the to square and the promotion piece if applicable
     * @note This function returns a null move if there is no move stored for the given hash position
     */
    static Move getMove(uint64_t hash);

    /**
     * @brief Gets a random book move
     * @param hash Zobrist hash representing current game state
     * @param board Board object representing current board state
     * @return Random book move if there exists a move in the book in the given position, otherwise a null move
     * @note This function does return the exact move including castling, en passant and capture flags
     */
    static Move getMove(uint64_t hash, Board& board);

private:
    inline static std::unordered_map<uint64_t, std::vector<Move>> book;
    inline static std::mt19937 rng{std::random_device{}()};
};

#endif // OPENING_BOOK_H