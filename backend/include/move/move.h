#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include "chess_types.h"

/**
 * @class Move
 * @brief Represents a single move
 * 
 * This class stores information about a move:
 * - Square that a piece previously occupied
 * - Square that a piece now occupies
 * - Flag representing the piece captured from the move if any
 * - Flag representing the piece gained from promotion if any
 * - Flag representing what type of castling was played in this move
 * - Flag representing if the move was an en passant capture
 * 
 * Flags are defined as follows:
 * 
 * - capturedPiece:
 *      - PAWN = 0
 *      - KNIGHT = 1
 *      - BISHOP = 2
 *      - ROOK = 3
 *      - QUEEN = 4
 *      - NO CAPTURE = NO_CAPTURE
 * 
 * - promotionPiece:
 *      - KNIGHT = 1
 *      - BISHOP = 2
 *      - ROOK = 3
 *      - QUEEN = 4
 *      - NO PROMOTION = NO_PROMOTION
 * 
 * - castling:
 *      - 0 = kingside castling
 *      - 1 = queenside castling
 *      - NO_CASTLE = No castling was played
 * 
 * enPassant:
 *      - 1 if en passant move was played
 *      - NO_EN_PASSANT if en passant move was not played
 */
class Move {
public:
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    inline static constexpr uint8_t NO_CAPTURE = 6; ///< Flag to represent moves does not capture
    inline static constexpr uint8_t NO_PROMOTION = 6; ///< Flag to represent move does not promote
    inline static constexpr uint8_t NO_CASTLE = 2; ///< Flag to represent move does not castle
    inline static constexpr uint8_t NO_EN_PASSANT = 0; ///< Flag to represent move does not en passant

    auto operator<=>(const Move& other) const = default;

    /**
     * @brief Initialise a Move object
     * @param fromSquare Square that the piece moves from
     * @param toSquare Square that the piece moves to
     * @param capturedPiece Flag representing which piece was captured
     * @param promotionPiece Flag representing which piece was gained from promotion
     * @param castling Flag representing which type of castling move was played
     * @param enPassant Flag representing if there was an en passant move played
     */
    Move(uint8_t fromSquare, uint8_t toSquare, uint8_t capturedPiece = NO_CAPTURE, uint8_t promotionPiece = NO_PROMOTION, 
            uint8_t castling = NO_CASTLE, uint8_t enPassant = NO_EN_PASSANT) : move(0) {

        setFromSquare(fromSquare);
        setToSquare(toSquare);
        setCapturedPiece(capturedPiece);
        setPromotionPiece(promotionPiece);
        setCastling(castling);
        setEnPassant(enPassant);
    }

    /**
     * @brief Gets the square that the piece moves from
     * @return Square that the piece moves from
     */
    inline constexpr uint8_t getFromSquare() const {
        return static_cast<uint8_t>(move & SQUARE_MASK);
    }

    /**
     * @brief Gets the square that the piece moves to
     * @return Square that the piece moves to
     */
    inline constexpr uint8_t getToSquare() const {
        return static_cast<uint8_t>((move >> TO_SHIFT) & SQUARE_MASK);
    }

    /**
     * @brief Gets the piece that was captured
     * @return uint8_t flag representing what piece was captured if any
     */
    inline constexpr uint8_t getCapturedPiece() const {
        return static_cast<uint8_t>((move >> CAPTURE_SHIFT) & PIECE_MASK);
    }

    /**
     * @brief Gets the piece that was gained from promotion
     * @return uint8_t flag representing what piece was gained from promotion if any
     */
    inline constexpr uint8_t getPromotionPiece() const {
        return static_cast<uint8_t>((move >> PROMOTION_SHIFT) & PIECE_MASK);
    }

    /**
     * @brief Gets the type of castling played in this move
     * @return uint8_t flag representing what type of castling was played in this move if any
     */
    inline constexpr uint8_t getCastling() const {
        return static_cast<uint8_t>((move >> CASTLING_SHIFT) & CASTLE_MASK);
    }

    /**
     * @brief Returns if the move was an en passant
     * @return uint8_t flag representing if the move was an en passant
     */
    inline constexpr uint8_t getEnPassant() const {
        return static_cast<uint8_t>((move >> EN_PASSANT_SHIFT) & EN_PASSANT_MASK);
    }

    /**
     * @param square Square that the piece moves from
     */
    inline void setFromSquare(uint8_t square) {
        move &= ~SQUARE_MASK;
        move |= static_cast<uint32_t>(square);
    }

    /**
     * @param square Square that the piece moves to
     */
    inline void setToSquare(uint8_t square) {
        move &= ~(SQUARE_MASK << TO_SHIFT);
        move |= (static_cast<uint32_t>(square) << TO_SHIFT);
    }

    /**
     * @param piece Piece flag representing what piece was captured if any
     */
    inline void setCapturedPiece(uint8_t piece) {
        move &= ~(PIECE_MASK << CAPTURE_SHIFT);
        move |= (static_cast<uint32_t>(piece) << CAPTURE_SHIFT);
    }

    /**
     * @param piece Piece flag representing what piece was gained from promotion if any
     */
    inline void setPromotionPiece(uint8_t piece) {
        move &= ~(PIECE_MASK << PROMOTION_SHIFT);
        move |= (static_cast<uint32_t>(piece) << PROMOTION_SHIFT);
    }

    /**
     * @param castle Castling flag representing what type of castling was played in this move if any
     */
    inline void setCastling(uint8_t castle) {
        move &= ~(CASTLE_MASK << CASTLING_SHIFT);
        move |= (static_cast<uint32_t>(castle) << CASTLING_SHIFT);
    }

    /**
     * @param enPassant En passant flag representing if this move was an en passant capture
     */
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