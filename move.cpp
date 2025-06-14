#include <vector>
#include <array>
#include <optional>
#include <cstdint>
#include <cassert>
#include "chess_types.h"
#include "board.h"
#include "move.h"

using Bitboard = Chess::Bitboard;
using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;

constexpr std::array<Bitboard, 64> knightMoveTable = []() {
    std::array<Bitboard, 64> table {};
    constexpr int offsets[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2},
                                    {-1, -2}, {-2, -1}, {-2, 1}, {-1 , 2}};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Bitboard moves = 0ULL;
            for (auto& offset : offsets) {
                int row = i + offset[0];
                int col = j + offset[1];
                if (0 <= row && row < 8 && 0 <= col && col < 8) {
                    moves |= (1ULL << (8 * row + col));                    
                }
            }
            table[8 * i + j] = moves;
        }
    }
    return table;
}();

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

    Bitboard piecesBitboard = board.getPiecesBitboard();
    int8_t direction = (colour == Colour::WHITE) ? 1 : -1;
    uint8_t nextSquare = currSquare + 8 * direction;

    // One square forward
    if ((piecesBitboard & (1ULL << nextSquare)) == 0) {
        moves.push_back(makeMove(piece, colour, currSquare, nextSquare));

        // Two squares forward
        nextSquare += 8 * direction;
        if (nextSquare < 64) {
            uint8_t firstUnmovedPawnSquare = 28 - 20 * direction;
            bool isUnmoved = firstUnmovedPawnSquare <= currSquare && currSquare < firstUnmovedPawnSquare + 8;
            if (isUnmoved && ((piecesBitboard & (1ULL << nextSquare)) == 0)) {
                moves.push_back(makeMove(piece, colour, currSquare, nextSquare));
            }
        }
    }

    Bitboard opposingBitboard = board.getOpposingBitboard(colour);

    // Left side pawn capture
    if (Board::getFile(currSquare) != 0) {
        uint8_t nextLeftSquare = currSquare + 8 * direction - 1;
        if ((opposingBitboard & (1ULL << nextLeftSquare)) != 0) {
            moves.push_back(makeMove(piece, colour, currSquare, nextLeftSquare, nextLeftSquare));
        }

        // En passant
        const std::optional<uint8_t> enPassantSquare = board.getEnPassantSquare();
        if (enPassantSquare && currSquare == (*enPassantSquare) + 1) {
            moves.push_back(makeMove(piece, colour, currSquare, (*enPassantSquare) + 8 * direction, enPassantSquare));
        }
    }

    // Right side pawn capture
    if (Board::getFile(currSquare) != 7) {
        uint8_t nextRightSquare = currSquare + 8 * direction + 1;
        if ((opposingBitboard & (1ULL << nextRightSquare)) != 0) {
            moves.push_back(makeMove(piece, colour, currSquare, nextRightSquare, nextRightSquare));
        }

        // En passant
        const std::optional<uint8_t> enPassantSquare = board.getEnPassantSquare();
        if (enPassantSquare && currSquare == (*enPassantSquare) - 1) {
            moves.push_back(makeMove(piece, colour, currSquare, (*enPassantSquare) + 8 * direction, enPassantSquare));
        }
    }
}

void MoveGenerator::legalKnightMoves(const Board& board, Piece piece, Colour colour, 
                                     uint8_t currSquare, std::vector<Move>& moves) {

    Bitboard precomputedMoveBitboard = knightMoveTable[currSquare];
    precomputedMoveBitboard &= ~board.getBitBoard(colour);
    Bitboard captureBitboard = precomputedMoveBitboard & board.getOpposingBitboard(colour);

    uint8_t bitIndex = 0;
    while (precomputedMoveBitboard) {
        if (precomputedMoveBitboard & 0x1) {
            std::optional<uint8_t> capture = (captureBitboard & 0x1) ? std::optional<uint8_t>(bitIndex) : std::nullopt;
            moves.push_back(makeMove(piece, colour, currSquare, bitIndex, capture));
        }
        precomputedMoveBitboard >>= 1;
        captureBitboard >>= 1;
        bitIndex++;
    }
}

void MoveGenerator::legalBishopMoves(const Board& board, Piece piece, Colour colour, 
                                     uint8_t currSquare, std::vector<Move>& moves) {

    Colour opposingColour = (colour == Colour::WHITE) ?
                            Colour::BLACK :
                            Colour::WHITE;
    
    constexpr int directions[4] = {7, 9, -9, -7}; // ↖, ↗, ↙, ↘
    constexpr uint8_t rankChecks[4] = {7, 7, 0, 0};
    constexpr uint8_t fileChecks[4] = {0, 7, 0, 7};

    for (int i = 0; i < 4; i++) {
        if (Board::getFile(currSquare) != fileChecks[i]) {
            uint8_t square = currSquare + directions[i];
            while (Board::getFile(square) != fileChecks[i] && Board::getRank(square) != rankChecks[i] && !board.getColour(square)) {
                moves.push_back(makeMove(piece, colour, currSquare, square));
                square += directions[i];
            }
            std::optional<Colour> finalSquareColour = board.getColour(square);
            if (!finalSquareColour || finalSquareColour == opposingColour) {
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
    Function functions[2] = {Board::getFile, Board::getRank};

    for (int i = 0; i < 4; i++) {
        if (functions[i & 0x1](currSquare) != boundaryChecks[i]) { // toggle between checking file and rank
            uint8_t square = currSquare + directions[i];
            while (functions[i & 0x1](square) != boundaryChecks[i] && !board.getColour(square)) {
                moves.push_back(makeMove(piece, colour, currSquare, square));
                square += directions[i];
            }
            std::optional<Colour> finalSquareColour = board.getColour(square);
            if (!finalSquareColour || finalSquareColour == opposingColour) {
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

}

Move MoveGenerator::makeMove(Chess::PieceType piece, Chess::PieceColour colour, 
                             uint8_t fromSquare, uint8_t toSquare, 
                             std::optional<uint8_t> captureSquare) {
    return {
        .piece = piece,
        .colour = colour,
        .fromSquare = fromSquare,
        .toSquare = toSquare,
        .captureSquare = captureSquare
    };
}