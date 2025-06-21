#ifndef PRECOMPUTE_MOVES_H
#define PRECOMPUTE_MOVES_H

#include <cstdint>
#include <array>
#include <vector>
#include <optional>
#include "board/board.h"
#include "move/move.h"
#include "chess_types.h"

namespace {
    using Bitboard = Chess::Bitboard;

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
                    int row = i + offset[0];
                    int col = j + offset[1];
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
}

class PrecomputeMoves {
public:
    using Bitboard = Chess::Bitboard;
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;
    using Move = ChessMove::Move;

    inline static constexpr std::array<Bitboard, 64> knightMoveTable = [] {
        std::array<std::array<int, 2>, 8> offsets = {{{1, 2}, {2, 1}, {2, -1}, {1, -2},
                                                    {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}}};
        return generateMoveTable(offsets);
    }();

    inline static constexpr std::array<Bitboard, 64> kingMoveTable = [] {
        std::array<std::array<int, 2>, 8> offsets = {{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                                    {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}};
        return generateMoveTable(offsets);
    }();

    inline static constexpr std::array<Bitboard, 64> whitePawnCaptureTable = [] {
        std::array<std::array<int, 2>, 8> offsets = {{{-1, 1}, {1, 1}}};
        return generateMoveTable(offsets);
    }();

    inline static constexpr std::array<Bitboard, 64> blackPawnCaptureTable = [] {
        std::array<std::array<int, 2>, 8> offsets = {{{-1, -1}, {1, -1}}};
        return generateMoveTable(offsets);
    }();
};

#endif // PRECOMPUTE_MOVES_H