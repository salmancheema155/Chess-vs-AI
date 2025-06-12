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

static constexpr std::array<Bitboard, 64> knightMoveTable = []() {
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
    switch (piece) {
        case Piece::PAWN:
            return legalPawnMoves(board, piece, colour, currSquare);
            break;
        case Piece::KNIGHT:
            return legalKnightMoves(board, piece, colour, currSquare);
            break;
        case Piece::BISHOP:
            return legalBishopMoves(board, piece, colour, currSquare);
            break;
        case Piece::ROOK:
            return legalRookMoves(board, piece, colour, currSquare);
            break;
        case Piece::QUEEN:
            return legalQueenMoves(board, piece, colour, currSquare);
            break;
        case Piece::KING:
            return legalKingMoves(board, piece, colour, currSquare);
            break;
        default:
            assert(false && "Piece must be either PAWN, KNIGHT, BISHOP, ROOK, QUEEN or KING");
    }

    return {};
}

std::vector<Move> MoveGenerator::legalPawnMoves(const Board& board, Piece piece, 
                                                Colour colour, uint8_t currSquare) {
    std::vector<Move> moves;
    Bitboard piecesBitboard = board.getPiecesBitboard();
    int8_t direction = (colour == Colour::WHITE) ? 1 : -1;
    uint8_t nextSquare = currSquare + 8 * direction;

    if ((piecesBitboard & (1ULL << nextSquare)) == 0) {
        moves.push_back(makeMove(piece, colour, currSquare, nextSquare));

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

    if ((currSquare & 0x7) != 0) {
        uint8_t nextLeftSquare = currSquare + 8 * direction - 1;
        if ((opposingBitboard & (1ULL << nextLeftSquare)) != 0) {
            moves.push_back(makeMove(piece, colour, currSquare, nextLeftSquare, nextLeftSquare));
        }

        const std::optional<uint8_t> enPassantSquare = board.getEnPassantSquare();
        if (enPassantSquare && currSquare == (*enPassantSquare) + 1) {
            moves.push_back(makeMove(piece, colour, currSquare, (*enPassantSquare) + 8 * direction, enPassantSquare));
        }
    }

    if ((currSquare & 0x7) != 7) {
        uint8_t nextRightSquare = currSquare + 8 * direction + 1;
        if ((opposingBitboard & (1ULL << nextRightSquare)) != 0) {
            moves.push_back(makeMove(piece, colour, currSquare, nextRightSquare, nextRightSquare));
        }

        const std::optional<uint8_t> enPassantSquare = board.getEnPassantSquare();
        if (enPassantSquare && currSquare == (*enPassantSquare) - 1) {
            moves.push_back(makeMove(piece, colour, currSquare, (*enPassantSquare) + 8 * direction, enPassantSquare));
        }
    }

    return moves;
}

std::vector<Move> MoveGenerator::legalKnightMoves(const Board& board, Piece piece, 
                                                  Colour colour, uint8_t currSquare) {
    std::vector<Move> moves;
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

    return moves;
}

std::vector<Move> MoveGenerator::legalBishopMoves(const Board& board, Piece piece, 
                                                  Colour colour, uint8_t currSquare) {
                                                    
}

std::vector<Move> MoveGenerator::legalRookMoves(const Board& board, Piece piece, 
                                                Colour colour, uint8_t currSquare) {

}

std::vector<Move> MoveGenerator::legalQueenMoves(const Board& board, Piece piece, 
                                                 Colour colour, uint8_t currSquare) {

}

std::vector<Move> MoveGenerator::legalKingMoves(const Board& board, Piece piece, 
                                                Colour colour, uint8_t currSquare) {

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