#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include "chess_types.h"

class Move {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    inline static constexpr uint8_t NO_CAPTURE = 7;
    inline static constexpr uint8_t NO_PROMOTION = 7;
    inline static constexpr uint8_t NO_CASTLE = 0;
    inline static constexpr uint8_t NO_EN_PASSANT = 0;

    auto operator<=>(const Move& other) const = default;

    Move(uint8_t fromSquare, uint8_t toSquare, uint8_t capturedPiece = NO_CAPTURE, uint8_t promotionPiece = NO_PROMOTION, 
            uint8_t castling = NO_CASTLE, uint8_t enPassant = NO_EN_PASSANT) : move(0) {

        setFromSquare(fromSquare);
        setToSquare(toSquare);
        setCapturedPiece(capturedPiece);
        setPromotionPiece(promotionPiece);
        setCastling(castling);
        setEnPassant(enPassant);
    }

    inline constexpr uint8_t getFromSquare() const {
        return static_cast<uint8_t>(move & SQUARE_MASK);
    }

    inline constexpr uint8_t getToSquare() const {
        return static_cast<uint8_t>((move >> TO_SHIFT) & SQUARE_MASK);
    }

    inline constexpr uint8_t getCapturedPiece() const {
        return static_cast<uint8_t>((move >> CAPTURE_SHIFT) & PIECE_MASK);
    }

    inline constexpr uint8_t getPromotionPiece() const {
        return static_cast<uint8_t>((move >> PROMOTION_SHIFT) & PIECE_MASK);
    }

    inline constexpr uint8_t getCastling() const {
        return static_cast<uint8_t>((move >> CASTLING_SHIFT) & CASTLE_MASK);
    }

    inline constexpr uint8_t getEnPassant() const {
        return static_cast<uint8_t>((move >> EN_PASSANT_SHIFT) & EN_PASSANT_MASK);
    }

    inline void setFromSquare(uint8_t square) {
        move &= ~SQUARE_MASK;
        move |= static_cast<uint32_t>(square);
    }

    inline void setToSquare(uint8_t square) {
        move &= ~(SQUARE_MASK << TO_SHIFT);
        move |= (static_cast<uint32_t>(square) << TO_SHIFT);
    }

    inline void setCapturedPiece(uint8_t piece) {
        move &= ~(PIECE_MASK << CAPTURE_SHIFT);
        move |= (static_cast<uint32_t>(piece) << CAPTURE_SHIFT);
    }

    inline void setPromotionPiece(uint8_t piece) {
        move &= ~(PIECE_MASK << PROMOTION_SHIFT);
        move |= (static_cast<uint32_t>(piece) << PROMOTION_SHIFT);
    }

    inline void setCastling(uint8_t castle) {
        move &= ~(CASTLE_MASK << CASTLING_SHIFT);
        move |= (static_cast<uint32_t>(castle) << CASTLING_SHIFT);
    }

    inline void setEnPassant(uint8_t enPassant) {
        move &= ~(EN_PASSANT_MASK << EN_PASSANT_SHIFT);
        move |= (static_cast<uint32_t>(enPassant) << EN_PASSANT_SHIFT);
    }

private:
    uint32_t move;

    inline static constexpr uint8_t TO_SHIFT = 6;
    inline static constexpr uint8_t CAPTURE_SHIFT = 12;
    inline static constexpr uint8_t PROMOTION_SHIFT = 15;
    inline static constexpr uint8_t CASTLING_SHIFT = 18;
    inline static constexpr uint8_t EN_PASSANT_SHIFT = 20;

    inline static constexpr uint32_t SQUARE_MASK = 0x3F; // 6 bits
    inline static constexpr uint32_t PIECE_MASK = 0x7; // 3 bits
    inline static constexpr uint32_t CASTLE_MASK = 0x3; // 2 bits
    inline static constexpr uint32_t EN_PASSANT_MASK = 0x1; // 1 bit
};

#endif // MOVE_H