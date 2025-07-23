#include <cstdint>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <bit>
#include "engine/transposition_table.h"

namespace {
    std::size_t roundDownToNearestPowerOfTwo(std::size_t n) {
        if (n == 0) return 0;
        return std::bit_floor(n);
    }
}

TranspositionTable::TranspositionTable(std::size_t size) : 
                    TT_SIZE(roundDownToNearestPowerOfTwo(size * 1024 * 1024 / sizeof(TTEntry))),
                    currentGeneration(0) {

    table.resize(TT_SIZE);
    clear();
};

void TranspositionTable::add(uint64_t key, const TTEntry& entry) {
    std::size_t index = key & (TT_SIZE - 1);
    if (table[index].generation != currentGeneration || entry.depth >= table[index].depth) {
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

void TranspositionTable::incrementGeneration() {
    currentGeneration++;
}

int16_t TranspositionTable::getGeneration() {
    return currentGeneration;
}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTEntry{});
}