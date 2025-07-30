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

            table[file] = mask;
        }

        return table;
    }();

    inline const std::array<std::array<uint64_t, 64>, 2> backwardPawnMaskTable = [] {
        std::array<std::array<uint64_t, 64>, 2> table;
        
        for (uint8_t colour = 0; colour < 2; colour++) {
            for (uint8_t square = 8; square < 56; square++) {
                uint8_t rank = square / 8;
                uint64_t mask = (colour == 0) ? 
                    (1ULL << (8 * (rank + 1))) - 1 : // 0s followed by 8 * (rank + 1) 1s
                    ~((1ULL << (8 * rank)) - 1); // 8 * (rank + 1) 1s followed by 0s

                mask &= isolatedPawnMaskTable[square % 8];
                table[colour][square] = mask;
            }
        }

        return table;
    }();

    inline const std::array<std::array<uint64_t, 64>, 2> connectedPawnMaskTable = [] {
        std::array<std::array<uint64_t, 64>, 2> table;

        for (uint8_t colour = 0; colour < 2; colour++) {
            for (uint8_t square = 8; square < 56; square++) {
                uint8_t rank = square / 8, file = square % 8;
                uint64_t mask = 0ULL;

                // Defending pawn on the left
                if (file > 0) {
                    uint8_t pawnDefenderSquare = (colour == 0) ?
                        square - 9 :
                        square + 7;
                    
                    mask |= (1ULL << pawnDefenderSquare);
                }

                // Defending pawn on the right
                if (file < 7) {
                    uint8_t pawnDefenderSquare = (colour == 0) ?
                        square - 7 :
                        square + 9;
                    
                    mask |= (1ULL << pawnDefenderSquare);
                }

                table[colour][square] = mask;
            }
        }

        return table;
    }();
};

#endif // ENGINE_PRECOMPUTE_H