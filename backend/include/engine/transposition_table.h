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
    TTFlag flag;
    Move bestMove;
};

class TranspositionTable {
public:
    TranspositionTable(std::size_t size);

    void add(uint64_t key, const TTEntry& entry);

    TTEntry* getEntry(uint64_t key);

    void clear();

private:
    const std::size_t TT_SIZE;
    std::vector<TTEntry> table;
};

#endif // TRANSPOSITION_TABLE_H