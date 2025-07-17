#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

#include <cstdint>
#include <type_traits>

namespace Chess {
    using Bitboard = uint64_t;

    /**
     * Enum representing different types of pieces
     */
    enum class PieceType : uint8_t {
        PAWN = 0,
        KNIGHT = 1,
        BISHOP = 2,
        ROOK = 3,
        QUEEN = 4,
        KING = 5,
        NONE = 6
    };

    /**
     * Enum representing different colours of pieces
     */
    enum class PieceColour : uint8_t {
        WHITE = 0,
        BLACK = 1,
        NONE = 2
    };

    /**
     * Enum representing different castling types
     */
    enum class Castling : uint8_t {
        KINGSIDE = 0,
        QUEENSIDE = 1,
        NONE = 2
    };

    /**
     * Converts an enum value to its corresponding integer value
     * @param item Enum value to convert
     * @return Corresponding integer value for the enum
     * @warning The enum value must have a corresponding integer value and 
     * integer values must be contiguous starting from 0
     */
    template <typename Enum>
    inline constexpr std::underlying_type_t<Enum> toIndex(Enum item) {
        static_assert(std::is_enum_v<Enum>, "toIndex requires an Enum type");
        return static_cast<std::underlying_type_t<Enum>>(item);
    }

    /**
     * Converts an integer value to a corresponding enum value
     * @param index Integer value
     * @return Corresponding enum value
     * @warning The enum value must have a corresponding integer value and 
     * integer values must be contiguous starting from 0
     */
    template <typename Enum>
    inline constexpr Enum fromIndex(std::underlying_type_t<Enum> index) {
        static_assert(std::is_enum_v<Enum>, "fromIndex requires an Enum type");
        return static_cast<Enum>(index);
    }
}

#endif // CHESS_TYPES_H