#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <array>
#include "chess_types.h"

using Bitboard = uint64_t;

/**
 * Class representing current board state
 * Stores positions of each existing piece, current player turn and castling rights
 * Provides functions to extract and modify this data appropriately
 */
class Board {
public:
    Board();

    /**
     * @brief Gets current player's turn
     * @return PieceColour enum representing player's turn
    */
    Chess::PieceColour getTurn();

    /**
     * @brief Returns if can kingside castle
     * @param colour Player colour
     * @return True if can kingside castle else False
    */
    bool getKingsideCastle(Chess::PieceColour colour);

    /**
     * @brief Checks if can queenside castle
     * @param colour Player colour
     * @return True if can queenside castle else False
    */
    bool getQueensideCastle(Chess::PieceColour colour);

    /**
     * @brief Prevents king's side castling
     * @param colour Player colour
    */
    void nullifyKingsideCastle(Chess::PieceColour colour);

    /**
     * @brief Prevents queen's side castling
     * @param colour Player colour
    */
    void nullifyQueensideCastle(Chess::PieceColour colour);

    /**
     * @brief Switches the current player turn to opposing player
    */
    void switchTurn();

    /**
     * Adds a piece to the board
     * @param type Type of piece e.g. PAWN
     * @param colour Colour of piece
     * @param square Square to add the piece to (0-63)
     */
    void addPiece(Chess::PieceType piece, Chess::PieceColour colour, uint8_t square);

    /**
     * Remove a piece from the board
     * @param type Type of piece e.g. PAWN
     * @param colour Colour of piece
     * @param square Square to remove the piece from (0-63)
     */
    void removePiece(Chess::PieceType piece, Chess::PieceColour colour, uint8_t square);

private:
    Chess::PieceColour currTurn;
    std::array<bool, 2> kingsideCastle;
    std::array<bool, 2> queensideCastle;

    // Indexed as [colour][pieceType]
    std::array<std::array<Bitboard, toIndex(Chess::PieceType::COUNT)>, 2> pieceBitboards;
};

#endif // BOARD_H