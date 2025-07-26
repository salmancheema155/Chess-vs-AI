#include <cstdint>
#include <cstddef>
#include <cassert>
#include <climits>
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
                    TT_SIZE(roundDownToNearestPowerOfTwo(size * 1024 * 1024 / sizeof(TTBucket))) {

    table.resize(TT_SIZE);
    clear();
};

void TranspositionTable::add(uint64_t key, const TTEntry& newEntry) {
    TTBucket& bucket = table[key & (TT_SIZE - 1)];

    for (uint8_t i = 0; i < TTBucket::BUCKET_SIZE; i++) {
        if (bucket.entries[i].zobristKey == key) {
            bucket.entries[i] = newEntry;
            return;
        }
    }

    uint8_t replaceIndex = 0;
    int minScore = std::numeric_limits<int>::max();

    for (uint8_t i = 0; i < TTBucket::BUCKET_SIZE; i++) {
        TTEntry& entry = bucket.entries[i];

        int16_t score = 0;
        score += entry.depth << 8;
        score -= (currentGeneration - entry.generation) << 6;
        if (entry.flag == TTFlag::EXACT) score += 128;

        if (score < minScore) {
            minScore = score;
            replaceIndex = i;
        }
    }

    bucket.entries[replaceIndex] = newEntry;
}

TTEntry* TranspositionTable::getEntry(uint64_t key) {
    TTBucket& bucket = table[key & (TT_SIZE - 1)];

    for (uint8_t i = 0; i < TTBucket::BUCKET_SIZE; i++) {
        if (bucket.entries[i].zobristKey == key) {
            return &bucket.entries[i];
        }
    }

    return nullptr;
}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTBucket{});
}