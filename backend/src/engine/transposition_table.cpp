#include <cstdint>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include "engine/transposition_table.h"

TranspositionTable::TranspositionTable(std::size_t size) : 
                    TT_SIZE(size * 1024 * 1024 / sizeof(TTEntry)) {

    assert(((TT_SIZE & (TT_SIZE - 1)) == 0) && "TT_SIZE must be a power of 2");
    table.resize(TT_SIZE);
    clear();
};

void TranspositionTable::add(uint64_t key, const TTEntry& entry) {
    std::size_t index = key & (TT_SIZE - 1);
    if (entry.depth >= table[index].depth) {
        table[index] = entry;
    }
}

TTEntry* TranspositionTable::getEntry(uint64_t key) {
    std::size_t index = key & (TT_SIZE - 1);
    if (table[index].zobristKey == key) {
        return &table[index];
    }

    return nullptr;
}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTEntry{});
}