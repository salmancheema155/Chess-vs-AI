#include <vector>
#include <array>
#include <optional>
#include <cstdint>
#include <bit>
#include <cassert>
#include "board/board.h"
#include "move/precompute_moves.h"
#include "move/move_generator.h"
#include "move/move.h"
#include "check/check.h"
#include "chess_types.h"

using Bitboard = Chess::Bitboard;
using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::toIndex;
using Chess::Castling;

namespace {
    /**
     * @brief Checks if a bit is set
     * @param num Number to check for the set bit
     * @param shift Bit index to check for the set bit starting from least significant bit
     */
    static inline constexpr bool bitSet(uint64_t num, uint8_t shift) {
        return num & (1ULL << shift);
    }

    /**
     * @brief Generates legal moves for a piece using a precomputed table of moves
     * @param board Board object representing current board state
     * @param piece Piece to find legal moves for
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal moves to
     * @param precomputedMoveBitboard Bitboard representation of possible moves for that piece (from table lookup)
     * @warning This function does not take into account king safety
     * It is possible that the moves generated from this function can put the king in direct danger
     * Use MoveGenerator::legalMoves instead to take into account king safety
     */
    static void legalMovesFromTable(const Board& board, Piece piece, Colour colour, uint8_t currSquare, 
                                    std::vector<Move>& moves, Bitboard precomputedMoveBitboard) {

        assert(currSquare < 64 && "currSquare must be between 0-63");
        precomputedMoveBitboard &= ~board.getBitboard(colour); // Remove moves which land onto same colour pieces
        Bitboard captureBitboard = precomputedMoveBitboard & board.getOpposingBitboard(colour); // Bitboard of moves which are captures

        while (precomputedMoveBitboard) {
            uint8_t bitIndex = std::countr_zero(precomputedMoveBitboard); // Square on board
            // Check if valid move is a capture (capture bit is set)
            uint8_t capture = bitSet(captureBitboard, bitIndex) ? toIndex(board.getPiece(bitIndex)) : Move::NO_CAPTURE;
            moves.push_back(Move(currSquare, bitIndex, capture));
            precomputedMoveBitboard &= precomputedMoveBitboard - 1; // Remove trailing set bit
        }
    }
}

std::vector<Move> MoveGenerator::legalMoves(Board& board, Piece piece, 
                                            Colour colour, uint8_t currSquare) {

    assert(currSquare < 64 && "currSquare must be between 0-63");
    std::vector<Move> moves;
    moves.reserve(16);
    switch (piece) {
        case Piece::PAWN:
            pseudoLegalPawnMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::KNIGHT:
            pseudoLegalKnightMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::BISHOP:
            pseudoLegalBishopMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::ROOK:
            pseudoLegalRookMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::QUEEN:
            pseudoLegalQueenMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::KING:
            pseudoLegalKingMoves(board, piece, colour, currSquare, moves);
            break;
        default:
            assert(false && "Piece must be either PAWN, KNIGHT, BISHOP, ROOK, QUEEN or KING");
            return {};
    }

    auto castlingRightsBeforeMove = board.getCastlingRights();
    auto enPassantSquareBeforeMove = board.getEnPassantSquare();
    for (int i = static_cast<int>(moves.size()) - 1; i >= 0; i--) {
        const Move& move = moves[i];
        board.makeMove(move, colour);
        bool inCheck = Check::isInCheck(board, colour);
        board.undo(move, colour, castlingRightsBeforeMove, enPassantSquareBeforeMove);

        if (inCheck) {
            moves.erase(moves.begin() + i);
        }
    }

    return moves;
}

std::vector<Move> MoveGenerator::legalMoves(Board& board, Colour colour) {
    std::vector<Move> moves;
    moves.reserve(256);
    constexpr Piece pieces[6] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, 
                                Piece::ROOK, Piece::QUEEN, Piece::KING};
    
    for (Piece piece : pieces) {
        Bitboard bitboard = board.getBitboard(piece, colour);
        while (bitboard) {
            uint8_t square = std::countr_zero(bitboard);
            std::vector<Move> pieceMoves = MoveGenerator::legalMoves(board, piece, colour, square);
            moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            bitboard &= (bitboard - 1);
        }
    }

    return moves;
}

void MoveGenerator::pseudoLegalPawnMoves(const Board& board, Piece piece, Colour colour, 
                                   uint8_t currSquare, std::vector<Move>& moves) {

    int8_t direction = (colour == Colour::WHITE) ? 1 : -1;
    uint8_t nextSquare = currSquare + 8 * direction;
    uint8_t promotionPawnRank = (colour == Colour::WHITE) ? 6 : 1;

    // One square forward
    if (board.isEmpty(nextSquare)) {
        // Possible promotion
        if (Board::getRank(currSquare) == promotionPawnRank) {
            moves.push_back(Move(currSquare, nextSquare, Move::NO_CAPTURE, toIndex(Piece::KNIGHT)));
            moves.push_back(Move(currSquare, nextSquare, Move::NO_CAPTURE, toIndex(Piece::BISHOP)));
            moves.push_back(Move(currSquare, nextSquare, Move::NO_CAPTURE, toIndex(Piece::ROOK)));
            moves.push_back(Move(currSquare, nextSquare, Move::NO_CAPTURE, toIndex(Piece::QUEEN)));
        } else {
            moves.push_back(Move(currSquare, nextSquare));

            // Two squares forward
            nextSquare += 8 * direction;
            uint8_t initialPawnRank = (colour == Colour::WHITE) ? 1 : 6;
            bool isUnmoved = (Board::getRank(currSquare) == initialPawnRank);
            if (isUnmoved && board.isEmpty(nextSquare)) { // Unmoved and square is free
                moves.push_back(Move(currSquare, nextSquare));
            }
        }
    }

    constexpr uint8_t files[2] = {0, 7}; // Boundaries for left and right captures respectively
    // Diagonal capture squares
    int captureSquares[2] = {
        static_cast<int> (currSquare + 8 * direction - 1), 
        static_cast<int> (currSquare + 8 * direction + 1)};
    constexpr int enPassantDirections[2] = {-1, 1}; // Left, right
    const std::optional<uint8_t> enPassantSquare = board.getEnPassantSquare();
    
    for (int i = 0; i < 2; i++) {
        // Diagonal capture
        if (Board::getFile(currSquare) != files[i]) {
            uint8_t captureSquare = captureSquares[i];
            // Opponent piece at capture square
            if (board.isOpponentOccupied(colour, captureSquare)) {
                uint8_t capture = toIndex(board.getPiece(captureSquare));

                // Possible promotion
                if (Board::getRank(currSquare) == promotionPawnRank) {
                    moves.push_back(Move(currSquare, captureSquare, capture, toIndex(Piece::KNIGHT)));
                    moves.push_back(Move(currSquare, captureSquare, capture, toIndex(Piece::BISHOP)));
                    moves.push_back(Move(currSquare, captureSquare, capture, toIndex(Piece::ROOK)));
                    moves.push_back(Move(currSquare, captureSquare, capture, toIndex(Piece::QUEEN)));
                } else {
                    moves.push_back(Move(currSquare, captureSquare, capture));
                }
            }

            // En passant
            // Check that pawn is adjacent to the en passant square
            if (enPassantSquare && currSquare + enPassantDirections[i] == *enPassantSquare) {
                moves.push_back(Move(currSquare, (*enPassantSquare) + 8 * direction, 
                                        toIndex(Piece::PAWN), Move::NO_PROMOTION, Move::NO_CASTLE, 1));
            }
        }
    }
}

void MoveGenerator::pseudoLegalKnightMoves(const Board& board, Piece piece, Colour colour, 
                                     uint8_t currSquare, std::vector<Move>& moves) {

    legalMovesFromTable(board, piece, colour, currSquare, moves, PrecomputeMoves::knightMoveTable[currSquare]);
}

void MoveGenerator::pseudoLegalBishopMoves(const Board& board, Piece piece, Colour colour, 
                                     uint8_t currSquare, std::vector<Move>& moves) {

    Colour opposingColour = (colour == Colour::WHITE) ?
                            Colour::BLACK :
                            Colour::WHITE;
    
    constexpr int directions[4] = {7, 9, -9, -7}; // ↖, ↗, ↙, ↘
    constexpr uint8_t rankChecks[4] = {7, 7, 0, 0}; // Boundaries depending on direction
    constexpr uint8_t fileChecks[4] = {0, 7, 0, 7}; // Boundaries depending on direction

    for (int i = 0; i < 4; i++) {
        if (Board::getFile(currSquare) != fileChecks[i] && Board::getRank(currSquare) != rankChecks[i]) {
            uint8_t square = currSquare + directions[i];
            // Not at edge of the board and square is empty
            while (Board::getFile(square) != fileChecks[i] && Board::getRank(square) != rankChecks[i] && board.isEmpty(square)) {
                moves.push_back(Move(currSquare, square));
                square += directions[i];
            }
            // Final square is either an empty square on the edge of the board or an occupied square
            Colour finalSquareColour = board.getColour(square);
            if (finalSquareColour == Colour::NONE || finalSquareColour == opposingColour) {
                // Capture if final square is of the opposing colour
                uint8_t capture = (finalSquareColour == opposingColour) ?
                                    toIndex(board.getPiece(square)) :
                                    Move::NO_CAPTURE;

                moves.push_back(Move(currSquare, square, capture));
            }
        }
    }
}

void MoveGenerator::pseudoLegalRookMoves(const Board& board, Piece piece, Colour colour, 
                                   uint8_t currSquare, std::vector<Move>& moves) {

    using Function = uint8_t(*)(uint8_t);
    Colour opposingColour = (colour == Colour::WHITE) ?
                        Colour::BLACK :
                        Colour::WHITE;
    
    constexpr int directions[4] = {-1, 8, 1, -8}; // ←, ↑, →, ↓
    constexpr uint8_t boundaryChecks[4] = {0, 7, 7, 0}; // file, rank, file, rank
    Function functions[2] = {Board::getFile, Board::getRank}; // Alternate between file and rank checks

    for (int i = 0; i < 4; i++) {
        if (functions[i & 0x1](currSquare) != boundaryChecks[i]) { // toggle between checking file and rank
            uint8_t square = currSquare + directions[i];
            // Not at edge of the board and square is empty
            while (functions[i & 0x1](square) != boundaryChecks[i] && board.isEmpty(square)) {
                moves.push_back(Move(currSquare, square));
                square += directions[i];
            }
            // Final square is either an empty square on the edge of the board or an occupied square
            Colour finalSquareColour = board.getColour(square);
            if (finalSquareColour == Colour::NONE || finalSquareColour == opposingColour) {
                // Capture if final square is of the opposing colour
                uint8_t capture = (finalSquareColour == opposingColour) ?
                                    toIndex(board.getPiece(square)) :
                                    Move::NO_CAPTURE;

                moves.push_back(Move(currSquare, square, capture));
            }
        }
    }
}

void MoveGenerator::pseudoLegalQueenMoves(const Board& board, Piece piece, Colour colour, 
                                    uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalBishopMoves(board, piece, colour, currSquare, moves);
    pseudoLegalRookMoves(board, piece, colour, currSquare, moves);
}

void MoveGenerator::pseudoLegalKingMoves(const Board& board, Piece piece, Colour colour, 
                                   uint8_t currSquare, std::vector<Move>& moves) {

    // Regular moves
    legalMovesFromTable(board, piece, colour, currSquare, moves, PrecomputeMoves::kingMoveTable[currSquare]);

    // Castling moves
    bool canQueensideCastle = board.getCastlingRights(colour, Castling::QUEENSIDE);
    bool canKingsideCastle = board.getCastlingRights(colour, Castling::KINGSIDE);

    if (canQueensideCastle) {
        uint64_t emptySquareMask = (colour == Colour::WHITE) ? 0xE : 0x0E00000000000000; // Squares between king and rook are empty
        if ((board.getPiecesBitboard() & emptySquareMask) == 0) {
            // Cannot pass through attacked square and cannot castle if in check
            if (!Check::isInDanger(board, colour, currSquare - 1) && !Check::isInCheck(board, colour)) {
                moves.push_back(Move(currSquare, currSquare - 2, Move::NO_CAPTURE, Move::NO_PROMOTION, toIndex(Castling::QUEENSIDE)));
            }
        }
    }

    if (canKingsideCastle) {
        uint64_t emptySquareMask = (colour == Colour::WHITE) ? 0x60 : 0x6000000000000000; // Squares between king and rook are empty
        if ((board.getPiecesBitboard() & emptySquareMask) == 0) {
            // Cannot pass through attacked square and cannot castle if in check
            if (!Check::isInDanger(board, colour, currSquare + 1) && !Check::isInCheck(board, colour)) {
                moves.push_back(Move(currSquare, currSquare + 2, Move::NO_CAPTURE, Move::NO_PROMOTION, toIndex(Castling::KINGSIDE)));
            }
        }
    }
}