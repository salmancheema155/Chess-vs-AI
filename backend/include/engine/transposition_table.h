#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "move/move.h"

enum class TTFlag : uint8_t {
    EXACT = 0,
    LOWER_BOUND = 1,
    UPPER_BOUND = 2
};

struct TTEntry {
    uint64_t zobristKey;
    int16_t eval;
    uint8_t depth;
    int16_t generation;
    TTFlag flag;
    Move bestMove;
};

class TranspositionTable {
public:
    /**
     * @brief Creates a transposition table
     * @param size Size of transposition table in MB
     * @note The size of the transposition table may be slightly less than the specified number due to rounding for optimisation
     */
    TranspositionTable(std::size_t size);

    /**
     * @brief Adds an entry to the table
     * @param key Key for table entry
     * @param entry Table entry
     * @note The entry may not be added if another entry occupies its place
     */
    void add(uint64_t key, const TTEntry& entry);

    /**
     * @brief Gets the entry from the table
     * @param key Key for table entry
     * @return Pointer to the entry if an entry exists at the key, otherwise returns nullptr
     */
    TTEntry* getEntry(uint64_t key);

    /**
     * @brief Increments the current generation
     */
    void incrementGeneration();

    /**
     * @brief Gets the current generation
     * @return Current generation
     */
    int16_t getGeneration();

    /**
     * @brief Clears all table entries
     */
    void clear();

private:
    const std::size_t TT_SIZE;
    std::vector<TTEntry> table;
    int16_t currentGeneration;
};

#endif // TRANSPOSITION_TABLE_H