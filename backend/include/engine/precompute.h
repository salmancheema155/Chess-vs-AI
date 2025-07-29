#ifndef ENGINE_PRECOMPUTE_H
#define ENGINE_PRECOMPUTE_H

#include <cstdint>
#include <array>

namespace EnginePrecompute {
    inline constexpr std::array<uint64_t, 8> isolatedPawnMaskTable = [] {
        std::array<uint64_t, 8> table;
        constexpr uint64_t leftMostFileMask = 0x0101010101010101ULL;

        for (uint8_t file = 0; file < 8; file++) {
            uint64_t mask = 0ULL;
            uint64_t fileMask = leftMostFileMask << file;

            // Add immediate right file
            if (file < 7) {
                mask |= (fileMask << 1);
            }

            // Add immediate left file
            if (file > 0) {
                mask |= (fileMask >> 1);
            }

            table[file] = fileMask;
        }

        return table;
    }();
};

#endif // ENGINE_PRECOMPUTE_H