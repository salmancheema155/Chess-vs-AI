#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <array>
#include <vector>
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
    inline Colour getTurn() const {
        return currTurn;
    }

    /**
     * @brief Gets bitboard representing current board state
     * @return Bitboard of current board
     */
    inline Bitboard getPiecesBitboard() const {
        return piecesBitboard;
    }

    /**
     * @brief Gets bitboard representing white pieces current board state
     * @return Bitboard of white pieces
     */
    inline Bitboard getWhitePiecesBitboard() const {
        return whitePiecesBitboard;
    }

    /**
     * @brief Gets bitboard representing black pieces current board state
     * @return Bitboard of black pieces
     */
    inline Bitboard getBlackPiecesBitboard() const {
        return blackPiecesBitboard;
    }

    /**
     * @brief Gets bitboard for specified colour
     * @param colour Colour of player
     * @return Bitboard representation of player's pieces
     */
    inline Bitboard getBitboard(Colour colour) const {
        return (colour == Colour::WHITE) ?
            whitePiecesBitboard :
            blackPiecesBitboard;
    }

    /**
     * @brief Gets bitboard for specified colour and piece
     * @param piece Piece to get bitboard for
     * @param colour Colour of piece
     * @return Bitboard representation of player's specified piece locations
     */
    inline Bitboard getBitboard(Piece piece, Colour colour) const {
        return pieceBitboards[toIndex(colour)][toIndex(piece)];
    }

    /**
     * @brief Gets the bitboard of the opposing colour's pieces
     * @param colour Colour of player
     * @return Bitboard of oppposing colour's pieces, e.g. if colour == WHITE,
     * returns black pieces bitboard
     * @note colour is the colour of the player, not the opponent
     */
    inline Bitboard getOpposingBitboard(Colour colour) const {
        return (colour == Colour::WHITE) ?
            blackPiecesBitboard :
            whitePiecesBitboard;
    }

    /**
     * @brief Gets the bitboard for a specified opposing colour's piece
     * @param piece Piece to get bitboard for
     * @param colour Colour of player
     * @return Bitboard representation of opponent's specified piece locations
     * @note colour is the colour of the player, not the opponent
     */
    inline Bitboard getOpposingBitboard(Piece piece, Colour colour) const {
        Colour opposingColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
        return pieceBitboards[toIndex(opposingColour)][toIndex(piece)];
    } 

    /**
     * @brief Gets the square of the pawn that just moved 2 steps forward
     * @return Square of the pawn that just moved 2 steps forward if it exists
     * else std::nullopt if the last move was not a pawn 2 forward
     */
    inline std::optional<uint8_t> getEnPassantSquare() const {
        return enPassantSquare;
    }

    /**
     * @brief Gets the colour of the piece that occupies the square
     * @param square Square of the piece (0-63)
     * @return Colour of piece that occupies that square if it exists
     * else returns std::nullopt
     */
    std::optional<Colour> getColour(uint8_t square) const;

    /**
     * @brief Gets the piece that occupies the square
     * @param square Square of the piece (0-63)
     * @return Colour of piece that occupies that square if it exists
     * else returns std::nullopt
     */
    std::optional<Piece> getPiece(uint8_t square) const;

    /**
     * @brief Gets the piece that occupies the square and the colour of the piece
     * @param square Square on the board (0-63)
     * @return std::pair with first element being the Piece and the second being the colour
     */
    std::pair<std::optional<Piece>, std::optional<Colour>> getPieceAndColour(uint8_t square) const;

    /**
     * @brief Gets the square that the king occupies
     * @param colour Colour of king piece
     * @return Square that the king occupies
     * @attention This functions assumes that there is only one king on the board and always
     * will return the king with the lower square number if there are multiple
     * @warning Undefined behaviour if there is no king on the board
     */
    inline uint8_t getKingSquare(Colour colour) const {
        return __builtin_ctzll(pieceBitboards[toIndex(colour)][toIndex(Piece::KING)]);
    }

    /**
     * @brief Gets the squares that a given type of piece of a given colour occupies
     * @param piece Piece to find squares for
     * @param colour Colour of piece
     * @return Vector of squares that the given piece of the given colour occupies
     */
    inline std::vector<uint8_t> getSquares(Piece piece, Colour colour) const {
        std::vector<uint8_t> squares;
        squares.reserve(10);
        Bitboard bitboard = getBitboard(piece, colour);
        
        while (bitboard) {
            squares.push_back(__builtin_ctzll(bitboard));
            bitboard &= bitboard - 1;
        }

        return squares;
    }

    /**
     * @brief Checks if a square is empty
     * @param square Square to check if it is empty (0-63)
     * @return True if square is empty, otherwise false
     */
    inline bool isEmpty(uint8_t square) const {
        return !(piecesBitboard & (1ULL << square));
    }

    /**
     * @brief Checks if a square is occupied
     * @param square Square to check if it is occupied (0-63)
     * @return True if square is occupied, otherwise false
     */
    inline bool isOccupied(uint8_t square) const {
        return piecesBitboard & (1ULL << square);
    }

    /**
     * @brief Checks if a square is occupied with a given piece
     * @param piece Piece to check if it occupies that square
     * @param square Square to check if it is occupied (0-63)
     * @return True if square is occupied, otherwise false
     */
    inline bool isOccupied(Piece piece, uint8_t square) const {
        Bitboard pieceOccupied = getBitboard(piece, Colour::WHITE) |
                                 getBitboard(piece, Colour::BLACK);

        return pieceOccupied & (1ULL << square);
    }

    /**
     * @brief Checks if a player occupies a square
     * @param colour Colour of player
     * @param square Square to check if it is occupied (0-63)
     * @return True if square is occupied by player with specified colour, otherwise false
     */
    inline bool isSelfOccupied(Colour colour, uint8_t square) const {
        return getBitboard(colour) & (1ULL << square);
    }

    /**
     * @brief Checks if a player occupies a square with a given piece
     * @param piece Piece to check if it occupies that square
     * @param colour Colour of piece
     * @param square Square to check if it is occupied (0-63)
     * @return True if square is occupied by the piece with specified colour, otherwise false
     */
    inline bool isSelfOccupied(Piece piece, Colour colour, uint8_t square) const {
        return getBitboard(piece, colour) & (1ULL << square);
    }

    /**
     * @brief Checks if opponent occupies a square
     * @param colour Colour of player
     * @param square Square to check if it is occupied (0-63)
     * @return True if square is occupied by opponent, otherwise false
     * @note colour is the colour of the player, not the opponent
     */
    inline bool isOpponentOccupied(Colour colour, uint8_t square) const {
        return getOpposingBitboard(colour) & (1ULL << square);
    }

    /**
     * @brief Checks if a player occupies a square with a given piece
     * @param piece Piece to check if it occupies that square
     * @param colour Colour of piece
     * @param square Square to check if it is occupied (0-63)
     * @return True if square is occupied by the piece with specified colour, otherwise false
     */
    inline bool isOpponentOccupied(Piece piece, Colour colour, uint8_t square) const {
        return getOpposingBitboard(piece, colour) & (1ULL << square);
    }

    /**
     * @brief Returns if can kingside castle
     * @param colour Player colour
     * @return True if can kingside castle else False
    */
    inline bool getKingsideCastle(Colour colour) const {
        return kingsideCastle[toIndex(colour)];
    }

    /**
     * @brief Checks if can queenside castle
     * @param colour Player colour
     * @return True if can queenside castle else False
    */
    inline bool getQueensideCastle(Colour colour) const {
        return queensideCastle[toIndex(colour)];
    }

    /**
     * @brief Prevents king's side castling
     * @param colour Player colour
    */
    inline void nullifyKingsideCastle(Colour colour) {
        kingsideCastle[toIndex(colour)] = false;
    }

    /**
     * @brief Prevents queen's side castling
     * @param colour Player colour
    */
    inline void nullifyQueensideCastle(Colour colour) {
        queensideCastle[toIndex(colour)] = false;
    }

    /**
     * @brief Switches the current player turn to opposing player
    */
    inline void switchTurn() {
        currTurn = (currTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    }

    /**
     * @brief Updates the square of the pawn that just moved 2 forward
     * if it exists otherwise updates with std::nullopt
     */
    inline void setEnPassantSquare(std::optional<uint8_t> square) {
        enPassantSquare = square;
    }

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

    /**
     * @brief Resets board back to initial state
     */
    void resetBoard();

    /**
     * @brief Gets the rank that a square is located in
     * @param square Square to find the rank of (0-63)
     * @return Rank of the square
     */
    static inline uint8_t getRank(uint8_t square) {
        return square >> 3;
    }

    /**
     * @brief Gets the file that the square is located in (as an integer)
     * @param square Square to find the file of (0-63)
     * @return File of the square
     */
    static inline uint8_t getFile(uint8_t square) {
        return square & 0x7;
    }

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

    /**
     * @brief Resets the pieces back to their original starting position
     * @warning Does not reset en passant information, castling rights or turn control
     */
    void resetPieces();
};

#endif // BOARD_H