#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <array>
#include <optional>
#include "chess_types.h"

/**
 * Class representing current board state
 * Stores positions of each existing piece, current player turn and castling rights
 * Provides functions to extract and modify this data appropriately
 */
class Board {
public:
    using Bitboard = Chess::Bitboard;
    using Piece = Chess::PieceType;
    using Colour = Chess::PieceColour;

    Board();

    /**
     * @brief Gets current player's turn
     * @return PieceColour enum representing player's turn
    */
    Colour getTurn() const;

    /**
     * @brief Gets bitboard representing current board state
     * @return Bitboard of current board
     */
    Bitboard getPiecesBitboard() const;

    /**
     * @brief Gets bitboard representing white pieces current board state
     * @return Bitboard of white pieces
     */
    Bitboard getWhitePiecesBitboard() const;

    /**
     * @brief Gets bitboard representing black pieces current board state
     * @return Bitboard of black pieces
     */
    Bitboard getBlackPiecesBitboard() const;

    /**
     * @brief Gets the bitboard of the opposing colour's pieces
     * @param colour Colour of player
     * @return Bitboard of oppposing colour's pieces, e.g. if colour == WHITE,
     * returns black pieces bitboard
     */
    Bitboard getOpposingBitboard(Colour colour) const;

    /**
     * @brief Gets the square of the pawn that just moved 2 steps forward
     * @return Square of the pawn that just moved 2 steps forward if it exists
     * else std::nullopt if the last move was not a pawn 2 forward
     */
    std::optional<uint8_t> getEnPassantSquare() const;

    /**
     * @brief Returns if can kingside castle
     * @param colour Player colour
     * @return True if can kingside castle else False
    */
    bool getKingsideCastle(Colour colour) const;

    /**
     * @brief Checks if can queenside castle
     * @param colour Player colour
     * @return True if can queenside castle else False
    */
    bool getQueensideCastle(Colour colour) const;

    /**
     * @brief Prevents king's side castling
     * @param colour Player colour
    */
    void nullifyKingsideCastle(Colour colour);

    /**
     * @brief Prevents queen's side castling
     * @param colour Player colour
    */
    void nullifyQueensideCastle(Colour colour);

    /**
     * @brief Switches the current player turn to opposing player
    */
    void switchTurn();

    /**
     * @brief Updates the square of the pawn that just moved 2 forward
     * if it exists otherwise updates with std::nullopt
     */
    void setEnPassantSquare(std::optional<uint8_t> square);

    /**
     * Adds a piece to the board
     * @param piece Type of piece e.g. PAWN
     * @param colour Colour of piece
     * @param square Square to add the piece to (0-63)
     */
    void addPiece(Piece piece, Colour colour, uint8_t square);

    /**
     * Remove a piece from the board
     * @param piece Type of piece e.g. PAWN
     * @param colour Colour of piece
     * @param square Square to remove the piece from (0-63)
     */
    void removePiece(Piece piece, Colour colour, uint8_t square);

    /**
     * Moves a piece from a square to another
     * @param piece Type of piece e.g. PAWN
     * @param colour Colour of piece
     * @param fromSquare Square to move the piece from (0-63)
     * @param toSquare Square to move the piece to (0-63)
     * @attention This function moves a piece and updates it on the board without 
     * checking if the move is valid
     */
    void movePiece(Piece piece, Colour colour, uint8_t fromSquare, uint8_t toSquare);

private:
    Colour currTurn;
    std::array<bool, 2> kingsideCastle;
    std::array<bool, 2> queensideCastle;
    std::optional<uint8_t> enPassantSquare;
    Bitboard whitePiecesBitboard;
    Bitboard blackPiecesBitboard;
    Bitboard piecesBitboard;

    // Indexed as [colour][pieceType]
    std::array<std::array<Bitboard, Chess::toIndex(Piece::COUNT)>, 2> pieceBitboards;

    void resetPieces();
};

#endif // BOARD_H