#include <vector>
#include <array>
#include <optional>
#include <cstdint>
#include <cassert>
#include "chess_types.h"
#include "board.h"
#include "move_generator.h"

using Bitboard = Chess::Bitboard;
using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;

namespace {
    inline Move makeMove(Chess::PieceType piece, Chess::PieceColour colour, 
                            uint8_t fromSquare, uint8_t toSquare, 
                            std::optional<uint8_t> captureSquare = std::nullopt) {
    return {
        .piece = piece,
        .colour = colour,
        .fromSquare = fromSquare,
        .toSquare = toSquare,
        .captureSquare = captureSquare
    };
}
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

    constexpr std::array<Bitboard, 64> generateKnightMoveTable() {
        std::array<std::array<int, 2>, 8> offsets = {{{1, 2}, {2, 1}, {2, -1}, {1, -2},
                                                    {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}}};
        return generateMoveTable(offsets);
    }

    constexpr std::array<Bitboard, 64> generateKingMoveTable() {
        std::array<std::array<int, 2>, 8> offsets = {{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                                    {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}};
        return generateMoveTable(offsets);
    }

    inline int bitScan(Bitboard bitboard) {
        return __builtin_ctzll(bitboard);
    }

    inline bool bitSet(uint64_t num, uint8_t shift) {
        return num & (1ULL << shift);
    }

    void legalMovesFromTable(const Board& board, Piece piece, Colour colour, uint8_t currSquare, 
                            std::vector<Move>& moves, Bitboard precomputedMoveBitboard) {

        precomputedMoveBitboard &= ~board.getBitBoard(colour); // Remove moves which land onto same colour pieces
        Bitboard captureBitboard = precomputedMoveBitboard & board.getOpposingBitboard(colour); // Bitboard of moves which are captures

        while (precomputedMoveBitboard) {
            uint8_t bitIndex = bitScan(precomputedMoveBitboard); // Square on board
            // Check if valid move is a capture (capture bit is set)
            std::optional<uint8_t> capture = bitSet(captureBitboard, bitIndex) ? std::optional<uint8_t>(bitIndex) : std::nullopt;
            moves.push_back(makeMove(piece, colour, currSquare, bitIndex, capture));
            precomputedMoveBitboard &= precomputedMoveBitboard - 1; // Remove trailing set bit
        }
    }

    constexpr std::array<Bitboard, 64> knightMoveTable = generateKnightMoveTable();
    constexpr std::array<Bitboard, 64> kingMoveTable = generateKingMoveTable();
}

std::vector<Move> MoveGenerator::legalMoves(const Board& board, Piece piece, 
                                            Colour colour, uint8_t currSquare) {
    std::vector<Move> moves;
    switch (piece) {
        case Piece::PAWN:
            legalPawnMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::KNIGHT:
            legalKnightMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::BISHOP:
            legalBishopMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::ROOK:
            legalRookMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::QUEEN:
            legalQueenMoves(board, piece, colour, currSquare, moves);
            break;
        case Piece::KING:
            legalKingMoves(board, piece, colour, currSquare, moves);
            break;
        default:
            assert(false && "Piece must be either PAWN, KNIGHT, BISHOP, ROOK, QUEEN or KING");
    }

    return moves;
}

void MoveGenerator::legalPawnMoves(const Board& board, Piece piece, Colour colour, 
                                   uint8_t currSquare, std::vector<Move>& moves) {

    int8_t direction = (colour == Colour::WHITE) ? 1 : -1;
    uint8_t nextSquare = currSquare + 8 * direction;

    // One square forward
    if (board.isEmpty(nextSquare)) {
        moves.push_back(makeMove(piece, colour, currSquare, nextSquare));

        // Two squares forward
        nextSquare += 8 * direction;
        if (nextSquare < 64) {
            uint8_t initialPawnRank = (colour == Colour::WHITE) ? 1 : 6;
            bool isUnmoved = Board::getRank(currSquare) == initialPawnRank;
            if (isUnmoved && board.isEmpty(nextSquare)) { // Unmoved and square is free
                moves.push_back(makeMove(piece, colour, currSquare, nextSquare));
            }
        }
    }

    constexpr uint8_t files[2] = {0, 7}; // Boundaries for left and right captures respectively
    uint8_t captureSquares[2] = {currSquare + 8 * direction - 1, currSquare + 8 * direction + 1}; // Diagonal capture squares
    constexpr uint8_t enPassantDirections[2] = {-1, 1}; // Left, right
    const std::optional<uint8_t> enPassantSquare = board.getEnPassantSquare();
    
    for (int i = 0; i < 2; i++) {
        // Diagonal capture
        if (Board::getFile(currSquare) != files[i]) {
            uint8_t captureSquare = captureSquares[i];
            // Opponent piece at capture square
            if (board.isOpponentOccupied(colour, captureSquare)) {
                moves.push_back(makeMove(piece, colour, currSquare, captureSquare, captureSquare));
            }

            // En passant
            // Check that pawn is adjacent to the en passant square
            if (enPassantSquare && currSquare + enPassantDirections[i] == *enPassantSquare) {
                moves.push_back(makeMove(piece, colour, currSquare, (*enPassantSquare) + 8 * direction, enPassantSquare));
            }
        }
    }
}

void MoveGenerator::legalKnightMoves(const Board& board, Piece piece, Colour colour, 
                                     uint8_t currSquare, std::vector<Move>& moves) {

    legalMovesFromTable(board, piece, colour, currSquare, moves, knightMoveTable[currSquare]);
}

void MoveGenerator::legalBishopMoves(const Board& board, Piece piece, Colour colour, 
                                     uint8_t currSquare, std::vector<Move>& moves) {

    Colour opposingColour = (colour == Colour::WHITE) ?
                            Colour::BLACK :
                            Colour::WHITE;
    
    constexpr int directions[4] = {7, 9, -9, -7}; // ↖, ↗, ↙, ↘
    constexpr uint8_t rankChecks[4] = {7, 7, 0, 0}; // Boundaries depending on direction
    constexpr uint8_t fileChecks[4] = {0, 7, 0, 7}; // Boundaries depending on direction

    for (int i = 0; i < 4; i++) {
        if (Board::getFile(currSquare) != fileChecks[i]) {
            uint8_t square = currSquare + directions[i];
            // Not at edge of the board and square is empty
            while (Board::getFile(square) != fileChecks[i] && Board::getRank(square) != rankChecks[i] && board.isEmpty(square)) {
                moves.push_back(makeMove(piece, colour, currSquare, square));
                square += directions[i];
            }
            // Final square is either an empty square on the edge of the board or an occupied square
            std::optional<Colour> finalSquareColour = board.getColour(square);
            if (!finalSquareColour || finalSquareColour == opposingColour) {
                // Capture if final square is of the opposing colour
                std::optional<uint8_t> capture = (finalSquareColour == opposingColour) ?
                                                std::optional<uint8_t>(square) :
                                                std::nullopt;
                moves.push_back(makeMove(piece, colour, currSquare, square, capture));
            }
        }
    }
}

void MoveGenerator::legalRookMoves(const Board& board, Piece piece, Colour colour, 
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
                moves.push_back(makeMove(piece, colour, currSquare, square));
                square += directions[i];
            }
            // Final square is either an empty square on the edge of the board or an occupied square
            std::optional<Colour> finalSquareColour = board.getColour(square);
            if (!finalSquareColour || finalSquareColour == opposingColour) {
                // Capture if final square is of the opposing colour
                std::optional<uint8_t> capture = (finalSquareColour == opposingColour) ?
                                                std::optional<uint8_t>(square) :
                                                std::nullopt;
                moves.push_back(makeMove(piece, colour, currSquare, square, capture));
            }
        }
    }
}

void MoveGenerator::legalQueenMoves(const Board& board, Piece piece, Colour colour, 
                                    uint8_t currSquare, std::vector<Move>& moves) {

    legalBishopMoves(board, piece, colour, currSquare, moves);
    legalRookMoves(board, piece, colour, currSquare, moves);
}

void MoveGenerator::legalKingMoves(const Board& board, Piece piece, Colour colour, 
                                   uint8_t currSquare, std::vector<Move>& moves) {

    legalMovesFromTable(board, piece, colour, currSquare, moves, kingMoveTable[currSquare]);
}