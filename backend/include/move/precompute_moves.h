#ifndef PRECOMPUTE_MOVES_H
#define PRECOMPUTE_MOVES_H

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <bit>
#include <optional>
#include "board/board.h"
#include "move/move.h"
#include "chess_types.h"

namespace {
    using Bitboard = Chess::Bitboard;

    /**
     * @brief Gets the number of set bits
     * @param num Number to calculate the number of set bits
     * @return Number of set bits of num
     */
    constexpr int countBits(uint64_t num) {
        int count = 0;
        while (num) {
            count++;
            num &= num - 1;
        }

        return count;
    }

    /**
     * @brief Given an array of offsets, generates a table of possible moves
     * @param offsets An array of offset values where each offset value is represented as [offsetX, offsetY]
     * @return Array of bitboard representations of possible moves
     * @attention These moves are not necessarily legal - they are just based upon offset values
     */
    template <size_t N>
    constexpr std::array<Bitboard, 64> generateMoveTable(const std::array<std::array<int, 2>, N>& offsets) {
        std::array<Bitboard, 64> table {};
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Bitboard moves = 0ULL;
                for (auto& offset : offsets) {
                    int row = i + offset[1];
                    int col = j + offset[0];
                    if (0 <= row && row < 8 && 0 <= col && col < 8) {
                        // Set square bit in bitboard
                        moves |= (1ULL << (8 * row + col));                    
                    }
                }
                table[8 * i + j] = moves;
            }
        }
        return table;
    }

    /**
     * @brief Gets the rook mask of potential moves not including edge squares
     * @param square Square that the rook occupies
     * @return Rook mask of potential moves
     */
    constexpr Bitboard generateRookMask(const uint8_t square) {
        // Does not include edge squares in masks
        constexpr uint64_t leftFileMask = 0x0001010101010100; // A2-A7
        constexpr uint64_t bottomRankMask = 0x7E; // B1-G1

        uint8_t rank = square / 8;
        uint8_t file = square % 8;

        Bitboard mask = 0ULL;
        mask |= (leftFileMask << file);
        mask |= (bottomRankMask << (8 * rank));
        mask &= ~(1ULL << square); // Remove square rook is currently on

        return mask;
    }

    /**
     * @brief Gets the bishop mask of potential moves not including edge squares
     * @param square Square that the bishop occupies
     * @return Bishop mask of potential moves
     */
    constexpr Bitboard generateBishopMask(const uint8_t square) {
        constexpr int directions[4] = {7, 9, -9, -7}; // ↖, ↗, ↙, ↘
        constexpr uint8_t rankChecks[4] = {7, 7, 0, 0}; // Boundaries depending on direction
        constexpr uint8_t fileChecks[4] = {0, 7, 0, 7}; // Boundaries depending on direction

        Bitboard mask = 0ULL;
        for (int i = 0; i < 4; i++) {
            // Not at specific edge of the board
            if (Board::getRank(square) != rankChecks[i] && Board::getFile(square) != fileChecks[i]) {
                uint8_t newSquare = square + directions[i];

                // Not at specific edge of the board
                while (Board::getRank(newSquare) != rankChecks[i] && Board::getFile(newSquare) != fileChecks[i]) {
                    mask |= (1ULL << newSquare);
                    newSquare += directions[i];
                }
            }
        }

        return mask;
    }

    /**
     * @brief Generates a table of index offsets for the rook move table
     * @return Table of index offsets for the rook move table
     */
    constexpr std::array<int, 64> generateRookMoveTableOffsets() {
        std::array<int, 64> table {};
        table[0] = 0;

        for (uint8_t square = 1; square < 64; square++) {
            Bitboard mask = generateRookMask(square - 1);
            int blockerBitboardPermutations = 1 << countBits(mask); // 2^n permutations for possible blocker bitboards
            table[square] = table[square - 1] + blockerBitboardPermutations;
        }

        return table;
    }

    /**
     * @brief Generates a table of index offsets for the bishop move table
     * @return Table of index offsets for the bishop move table
     */
    constexpr std::array<int, 64> generateBishopMoveTableOffsets() {
        std::array<int, 64> table {};
        table[0] = 0;

        for (uint8_t square = 1; square < 64; square++) {
            Bitboard mask = generateBishopMask(square - 1);
            int blockerBitboardPermutations = 1 << countBits(mask); // 2^n permutations for possible blocker bitboards
            table[square] = table[square - 1] + blockerBitboardPermutations;
        }

        return table;
    }

    /**
     * @brief Generates a bitboard of rook moves including the final squares in each ray
     * @param square Square that the rook occupies
     * @param blockerBitboard Bitboard of blocking pieces in rays
     * @return Bitboard of rook moves
     */
    Bitboard generateRookMoves(const uint8_t square, const Bitboard blockerBitboard) {
        using Function = uint8_t(*)(uint8_t);

        constexpr int directions[4] = {-1, 8, 1, -8}; // ←, ↑, →, ↓
        constexpr uint8_t boundaryChecks[4] = {0, 7, 7, 0}; // file, rank, file, rank
        Function functions[2] = {Board::getFile, Board::getRank}; // Alternate between file and rank checks

        Bitboard moves = 0ULL;
        for (int i = 0; i < 4; i++) {
            // Not at specific edge of the board
            if (functions[i & 0x1](square) != boundaryChecks[i]) { // toggle between checking file and rank
                uint8_t newSquare = square + directions[i];
                // Not at specific edge of the board and square is not occupied
                while (functions[i & 0x1](newSquare) != boundaryChecks[i] && !(blockerBitboard & (1ULL << newSquare))) {
                    moves |= (1ULL << newSquare);
                    newSquare += directions[i];
                }
                // Final square is either an empty square on the edge of the board or an occupied square
                moves |= (1ULL << newSquare);
            }
        }

        return moves;
    }

    /**
     * @brief Generates a bitboard of bishop moves including the final squares in each ray
     * @param square Square that the bishop occupies
     * @param blockerBitboard Bitboard of blocking pieces in rays
     * @return Bitboard of bishop moves
     */
    Bitboard generateBishopMoves(const uint8_t square, const Bitboard blockerBitboard) {
        constexpr int directions[4] = {7, 9, -9, -7}; // ↖, ↗, ↙, ↘
        constexpr uint8_t rankChecks[4] = {7, 7, 0, 0}; // Boundaries depending on direction
        constexpr uint8_t fileChecks[4] = {0, 7, 0, 7}; // Boundaries depending on direction

        Bitboard moves = 0ULL;
        for (int i = 0; i < 4; i++) {
            // Not at specific edge of the board
            if (Board::getFile(square) != fileChecks[i] && Board::getRank(square) != rankChecks[i]) {
                uint8_t newSquare = square + directions[i];
                // Not at specific edge of the board and square is not occupied
                while (Board::getFile(newSquare) != fileChecks[i] && 
                       Board::getRank(newSquare) != rankChecks[i] && 
                       !(blockerBitboard & (1ULL << newSquare))) {

                    moves |= (1ULL << newSquare);
                    newSquare += directions[i];
                }
                // Final square is either an empty square on the edge of the board or an occupied square
                moves |= (1ULL << newSquare);
            }
        }

        return moves;
    }

    /**
     * @brief Converts a pext number to a blocker bitboard
     * @param pextIndex Pext number to convert
     * @param mask Mask of potential moves for sliding piece
     * @return Blocker bitboard from pext number
     */
    Bitboard pextIndexToBlockerBitboard(uint64_t pextIndex, Bitboard mask) {
        Bitboard blockerBitboard = 0ULL;
        int pextBitIndex = 0;

        while (mask) {
            int maskBitIndex = std::countr_zero(mask);
            uint64_t bit = (pextIndex >> pextBitIndex) & 0x1; // Gets the pextIndexBit of pextIndex
            blockerBitboard |= (bit << maskBitIndex); // Set the maskBitIndex of blockerBitboard to bit

            pextBitIndex++;
            mask &= mask - 1;
        }

        return blockerBitboard;
    }

    /**
     * @brief Converts a bitboard to a pext number
     * @param bitboard Bitboard to convert to pext number
     * @param mask Mask of potential moves for sliding piece
     * @return Pext number from blocker bitboard
     */
    inline uint64_t bitboardToPextIndex(Bitboard bitboard, Bitboard mask) {
        uint64_t pextIndex = 0ULL;
        int pextBitIndex = 0;

        while (mask) {
            int maskBitIndex = std::countr_zero(mask);
            uint64_t bit = (bitboard >> maskBitIndex) & 0x1; // Gets the maskBitIndex of bitboard
            pextIndex |= (bit << pextBitIndex); // Set the pextBitIndex of pextIndex to bit

            pextBitIndex++;
            mask &= mask - 1;
        }

        return pextIndex;
    }
}

class PrecomputeMoves {
public:
    using Bitboard = Chess::Bitboard;
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    inline static constexpr std::array<Bitboard, 64> knightMoveTable = [] {
        constexpr std::array<std::array<int, 2>, 8> offsets = {{{1, 2}, {2, 1}, {2, -1}, {1, -2},
                                                                {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}}};
        return generateMoveTable(offsets);
    }();

    inline static constexpr std::array<Bitboard, 64> kingMoveTable = [] {
        constexpr std::array<std::array<int, 2>, 8> offsets = {{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                                                {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}};
        return generateMoveTable(offsets);
    }();

    inline static constexpr std::array<std::array<Bitboard, 64>, 2> pawnThreatTable = [] {
        std::array<std::array<Bitboard, 64>, 2> table {};
        constexpr std::array<std::array<int, 2>, 8> whiteOffsets = {{{-1, -1}, {1, -1}}};
        constexpr std::array<std::array<int, 2>, 8> blackOffsets = {{{-1, 1}, {1, 1}}};

        table[0] = generateMoveTable(whiteOffsets);
        table[1] = generateMoveTable(blackOffsets);

        return table;
    }();

    inline static constexpr std::array<std::array<Bitboard, 64>, 2> pawnCaptureTable = [] {
        std::array<std::array<Bitboard, 64>, 2> table {};
        constexpr std::array<std::array<int, 2>, 8> whiteOffsets = {{{-1, 1}, {1, 1}}};
        constexpr std::array<std::array<int, 2>, 8> blackOffsets = {{{-1, -1}, {1, -1}}};

        table[0] = generateMoveTable(whiteOffsets);
        table[1] = generateMoveTable(blackOffsets);

        return table;
    }();

    /// Indexed as [white|black][square]
    inline static constexpr std::array<std::array<Bitboard, 64>, 2> singlePawnPushTable = [] {
        std::array<std::array<Bitboard, 64>, 2> table {};
        Colour colours[] = {Colour::WHITE, Colour::BLACK};

        for (Colour colour : colours) {
            for (int square = 0; square < 64; square++) {
                uint8_t rank = Board::getRank(square);
                int c = Chess::toIndex(colour);

                if (rank == 0 || rank == 7) {
                    table[c][square] = 0ULL;
                    continue;
                }

                int nextSquare = (c == 0) ? square + 8 : square - 8;
                table[c][square] = 1ULL << nextSquare;
            }
        }

        return table;
    }();

    /// Indexed as [white|black][square]
    inline static constexpr std::array<std::array<Bitboard, 64>, 2> doublePawnPushTable = [] {
        std::array<std::array<Bitboard, 64>, 2> table {};
        Colour colours[] = {Colour::WHITE, Colour::BLACK};

        for (Colour colour : colours) {
            for (int square = 0; square < 64; square++) {
                uint8_t rank = Board::getRank(square);
                int c = Chess::toIndex(colour);
                if (colour == Colour::WHITE && rank != 1) {
                    table[c][square] = 0ULL;
                    continue;
                }
                if (colour == Colour::BLACK && rank != 6) {
                    table[c][square] = 0ULL;
                    continue;
                }

                int doublePushSquare = (c == 0) ? square + 16 : square - 16;
                table[c][square] = 1ULL << doublePushSquare;
            }
        }

        return table;
    }();

    inline static constexpr std::array<std::array<Bitboard, 64>, 2> enPassantSquareTable = [] {
        std::array<std::array<Bitboard, 64>, 2> table {};
        Colour colours[] = {Colour::WHITE, Colour::BLACK};

        for (Colour colour : colours) {
            for (int square = 0; square < 64; square++) {
                uint8_t rank = Board::getRank(square);
                int c = Chess::toIndex(colour);
                
                if (colour == Colour::WHITE && rank != 4) {
                    table[c][square] = 0ULL;
                    continue;
                }
                if (colour == Colour::BLACK && rank != 3) {
                    table[c][square] = 0ULL;
                    continue;
                }

                uint8_t file = Board::getFile(square);
                Bitboard bitboard = 0ULL;

                if (file > 0) {
                    bitboard |= (1ULL << (square - 1));
                }

                if (file < 7) {
                    bitboard |= (1ULL << (square + 1));
                }
                
                table[c][square] = bitboard;
            }
        }

        return table;
    }();

    /**
     * @brief Gets a bitboard of pseudolegal rook moves including the final squares in each ray
     * @param square Square that the rook occupies
     * @param occupiedBitboard Bitboard representing all pieces on the board
     * @return Bitboard representation of rook moves
     */
    inline static Bitboard getRookMovesFromTable(const uint8_t square, const Bitboard occupiedBitboard) {
        Bitboard rookMask = rookMasks[square];
        int pextIndex = bitboardToPextIndex(occupiedBitboard, rookMask);
        int index = rookMoveTableOffsets[square] + pextIndex;
        
        return rookMoveTable[index];
    }

    /**
     * @brief Gets a bitboard of pseudolegal bishop moves including the final squares in each ray
     * @param square Square that the bishop occupies
     * @param occupiedBitboard Bitboard representing all pieces on the board
     * @return Bitboard representation of bishop moves
     */
    inline static Bitboard getBishopMovesFromTable(const uint8_t square, const Bitboard occupiedBitboard) {
        Bitboard bishopMask = bishopMasks[square];
        int pextIndex = bitboardToPextIndex(occupiedBitboard, bishopMask);
        int index = bishopMoveTableOffsets[square] + pextIndex;
        
        return bishopMoveTable[index];
    }

private:
    inline static constexpr std::array<int, 64> rookMoveTableOffsets = generateRookMoveTableOffsets();
    inline static constexpr std::size_t rookMoveTableSize = rookMoveTableOffsets[63] + (1ULL << countBits(generateRookMask(63)));

    inline static constexpr std::array<int, 64> bishopMoveTableOffsets = generateBishopMoveTableOffsets();
    inline static constexpr std::size_t bishopMoveTableSize = bishopMoveTableOffsets[63] + (1ULL << countBits(generateBishopMask(63)));

    inline static constexpr std::array<Bitboard, 64> rookMasks = [] {
        std::array<Bitboard, 64> table {};

        for (int square = 0; square < 64; square++) {
            table[square] = generateRookMask(square);
        }

        return table;
    }();

    inline static constexpr std::array<Bitboard, 64> bishopMasks = [] {
        std::array<Bitboard, 64> table {};

        for (int square = 0; square < 64; square++) {
            table[square] = generateBishopMask(square);
        }

        return table;
    }();

    inline static const std::array<Bitboard, rookMoveTableSize> rookMoveTable = [] {
        std::array<Bitboard, rookMoveTableSize> table {};

        for (uint8_t square = 0; square < 64; square++) {
            Bitboard mask = rookMasks[square];
            int blockerBitboardPermutations = 1 << std::popcount(mask); // 2^n permutations for possible blocker bitboards

            // Loop through each blocker bitboard permutation
            for (int pextIndex = 0; pextIndex < blockerBitboardPermutations; pextIndex++) {
                Bitboard blockers = pextIndexToBlockerBitboard(pextIndex, mask);
                Bitboard moves = generateRookMoves(square, blockers);
                int index = rookMoveTableOffsets[square] + pextIndex;
                table[index] = moves;
            }
        }

        return table;
    }();

    inline static const std::array<Bitboard, bishopMoveTableSize> bishopMoveTable = [] {
        std::array<Bitboard, bishopMoveTableSize> table {};

        for (uint8_t square = 0; square < 64; square++) {
            Bitboard mask = bishopMasks[square];
            int blockerBitboardPermutations = 1 << std::popcount(mask); // 2^n permutations for possible blocker bitboards

            // Loop through each blocker bitboard permutation
            for (int pextIndex = 0; pextIndex < blockerBitboardPermutations; pextIndex++) {
                Bitboard blockers = pextIndexToBlockerBitboard(pextIndex, mask);
                Bitboard moves = generateBishopMoves(square, blockers);
                int index = bishopMoveTableOffsets[square] + pextIndex;
                table[index] = moves;
            }
        }

        return table;
    }();
};

#endif // PRECOMPUTE_MOVES_H