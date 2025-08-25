#ifndef ENGINE_PRECOMPUTE_H
#define ENGINE_PRECOMPUTE_H

#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>

namespace EnginePrecompute {
    inline constexpr std::array<uint64_t, 8> adjacentFileMaskTable = [] {
        std::array<uint64_t, 8> table {};
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
        std::array<std::array<uint64_t, 64>, 2> table {};
        
        for (uint8_t colour = 0; colour < 2; colour++) {
            for (uint8_t square = 8; square < 56; square++) {
                uint8_t rank = square / 8;
                // Behind ranks
                uint64_t mask = (colour == 0) ? 
                    (1ULL << (8 * rank)) - 1 : // 0s followed by 8 * rank 1s
                    ~((1ULL << (8 * (rank + 1))) - 1); // 8 * (7 - rank) 1s followed by 0s

                mask &= adjacentFileMaskTable[square % 8]; // Only include adjacent files
                table[colour][square] = mask;
            }
        }

        return table;
    }();

    inline const std::array<std::array<uint64_t, 64>, 2> pawnChainMaskTable = [] {
        std::array<std::array<uint64_t, 64>, 2> table {};

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

    inline const std::array<std::array<uint64_t, 64>, 2> majorPawnShieldTable = [] {
        std::array<std::array<uint64_t, 64>, 2> table {};

        for (uint8_t colour = 0; colour < 2; colour++) {
            for (uint8_t square = 0; square < 64; square++) {
                uint8_t rank = square / 8, file = square % 8;
                uint64_t mask = 0ULL;

                // Not at first 2 ranks
                if (colour == 0 && rank != 0 && rank != 1) continue;
                if (colour == 1 && rank != 6 && rank != 7) continue;

                // Pawn directly in front of king
                if (file <= 2 || file >= 5) {
                    uint8_t inFrontPawnDefenderSquare = (colour == 0) ? square + 8 : square - 8;
                    mask |= (1ULL << inFrontPawnDefenderSquare);
                }

                table[colour][square] = mask;
            }
        }

        return table;
    }();

    inline const std::array<std::array<uint64_t, 64>, 2> minorPawnShieldTable = [] {
        std::array<std::array<uint64_t, 64>, 2> table {};

        for (uint8_t colour = 0; colour < 2; colour++) {
            for (uint8_t square = 0; square < 64; square++) {
                uint8_t rank = square / 8, file = square % 8;
                uint64_t mask = 0ULL;

                // Not at first rank or in central files
                if (colour == 0 && (rank != 0 || file >= 3 && file <= 5)) continue;
                if (colour == 1 && (rank != 7 || file >= 3 && file <= 5)) continue;

                // Pawn on the left diagonal of the king
                if (file > 0 && (file <= 2 || file >= 6)) {
                    uint8_t leftDiagonalPawnDefenderSquare = (colour == 0) ? square + 7 : square - 9;
                    mask |= (1ULL << leftDiagonalPawnDefenderSquare);
                }

                // Pawn on the right diagonal of the king
                if (file < 7 && (file <= 1 || file >= 4)) {
                    uint8_t rightDiagonalPawnDefenderSquare = (colour == 0) ? square + 9 : square - 7;
                    mask |= (1ULL << rightDiagonalPawnDefenderSquare);
                }

                table[colour][square] = mask;
            }
        }

        return table;
    }();

    inline const std::array<std::array<uint64_t, 64>, 2> passedPawnMaskTable = [] {
        std::array<std::array<uint64_t, 64>, 2> table {};

        for (uint8_t colour = 0; colour < 2; colour++) {
            for (uint8_t square = 0; square < 64; square++) {
                uint8_t rank = square / 8, file = square % 8;
                uint64_t mask = 0x0101010101010101ULL << file; // Mask of current file
                mask |= adjacentFileMaskTable[file]; // Add adjacent files

                // Remove current and behind ranks
                mask &= (colour == 0) ? ~((1ULL << (8 * (rank + 1))) - 1) : (1ULL << (8 * rank)) - 1;

                table[colour][square] = mask;
            }
        }

        return table;
    }();

    inline const std::array<uint64_t, 64> openFileTable = [] {
        std::array<uint64_t, 64> table {};

        for (uint8_t square = 0; square < 64; square++) {
            uint8_t file = square % 8;
            uint64_t mask = 0x0101010101010101ULL << file; // Mask of current file
            mask &= ~(1ULL << square); // Remove current square bit

            table[square] = mask;
        }

        return table;
    }();

    inline const std::array<std::array<uint64_t, 64>, 64> sameFileSquaresBetweenTable = [] {
        std::array<std::array<uint64_t, 64>, 64> table {};

        for (uint8_t i = 0; i < 64; i++) {
            for (uint8_t j = 0; j < 64; j++) {
                uint8_t iFile = i % 8;
                uint8_t jFile = j % 8;
                uint64_t mask = 0ULL;

                if (iFile != jFile) continue;

                uint8_t currSquare = std::min(i, j) + 8;
                uint8_t endSquare = std::max(i, j);
                while (currSquare < endSquare) {
                    mask |= (1ULL << currSquare);
                    currSquare += 8;
                }

                table[i][j] = mask;
            }
        }

        return table;
    }();

    inline const std::array<std::array<uint64_t, 64>, 64> sameRankSquaresBetweenTable = [] {
        std::array<std::array<uint64_t, 64>, 64> table {};

        for (uint8_t i = 0; i < 64; i++) {
            for (uint8_t j = 0; j < 64; j++) {
                uint8_t iRank = i / 8;
                uint8_t jRank = j / 8;
                uint64_t mask = 0ULL;

                if (iRank != jRank) continue;

                uint8_t currSquare = std::min(i, j) + 1;
                uint8_t endSquare = std::max(i, j);
                while (currSquare < endSquare) {
                    mask |= (1ULL << currSquare);
                    currSquare++;
                }

                table[i][j] = mask;
            }
        }

        return table;
    }();

    inline const std::array<std::array<uint8_t, 64>, 64> chebyshevDistanceTable = [] {
        std::array<std::array<uint8_t, 64>, 64> table {};

        for (uint8_t i = 0; i < 64; i++) {
            for (uint8_t j = 0; j < 64; j++) {
                int iRank = i / 8, iFile = i % 8;
                int jRank = j / 8, jFile = j % 8;

                // Chebyshev distance (maximum of horizontal and vertical distances)
                uint8_t rankDifference = static_cast<uint8_t>(std::abs(iRank - jRank));
                uint8_t fileDifference = static_cast<uint8_t>(std::abs(iFile - jFile));
                table[i][j] = std::max(rankDifference, fileDifference);
            }
        }

        return table;
    }();
};

#endif // ENGINE_PRECOMPUTE_H