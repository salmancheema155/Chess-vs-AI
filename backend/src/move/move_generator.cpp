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
     * @brief Generates pseudo legal moves using a precomputed table of moves
     * @param board Board object representing current board state
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal moves to
     * @param precomputedMoveBitboard Bitboard representation of possible moves for that piece (from table lookup)
     * @warning This function does not take into account king safety
     * It is possible that the moves generated from this function can put the king in direct danger
     * Use MoveGenerator::legalMoves instead to take into account king safety
     */
    static void pseudoLegalMovesFromTable(const Board& board, Colour colour, uint8_t currSquare, 
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

    /**
     * @brief Generates pseudo legal capture moves using a precomputed table of moves
     * @param board Board object representing current board state
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal capture moves to
     * @param precomputedMoveBitboard Bitboard representation of possible capture for that piece (from table lookup)
     * @warning This function does not take into account king safety
     * It is possible that the capture moves generated from this function can put the king in direct danger
     * Use MoveGenerator::legalCaptures instead to take into account king safety
     */
    static void pseudoLegalCapturesFromTable(const Board& board, Colour colour, uint8_t currSquare,
                                            std::vector<Move>& moves, Bitboard precomputedMoveBitboard) {

        assert(currSquare < 64 && "currSquare must be between 0-63");
        Bitboard captureBitboard = precomputedMoveBitboard & board.getOpposingBitboard(colour); // Bitboard of moves which are captures

        while (captureBitboard) {
            uint8_t bitIndex = std::countr_zero(captureBitboard); // Square on board
            uint8_t capture = toIndex(board.getPiece(bitIndex));
            moves.push_back(Move(currSquare, bitIndex, capture));
            captureBitboard &= captureBitboard - 1; // Remove trailing set bit
        }
    }

    /**
     * @brief Generates pseudo legal check moves which are not captures using a precomputed table of moves
     * @param board Board object representing current board state
     * @param colour Colour of piece
     * @param currSquare Square that the piece is located on (0-63)
     * @param moves Vector to append legal capture moves to
     * @param precomputedMoveBitboard Bitboard representation of possible moves for that piece (from table lookup)
     * @param precomputedKingThreatBitboard Bitboard representation of squares that attack the opposing colour king
     * @warning This function does not take into account king safety
     * It is possible that the capture moves generated from this function can put the king in direct danger
     * @attention This function does not take into account discovered attack checks
     * @note This function does not take into account pseudo legal moves where a king attacks another king
     */
    static void pseudoLegalNonCaptureChecksFromTable(const Board& board, Colour colour, uint8_t currSquare, std::vector<Move>& moves, 
                                                    Bitboard precomputedMoveBitboard, Bitboard precomputedKingThreatBitboard) {

        assert(currSquare < 64 && "currSquare must be between 0-63");
        Bitboard captureBitboard = precomputedMoveBitboard & board.getOpposingBitboard(colour); // Bitboard of moves which are captures
        precomputedMoveBitboard &= ~board.getBitboard(colour); // Remove moves which land onto same colour pieces
        precomputedMoveBitboard &= ~captureBitboard; // Remove moves which are captures
        precomputedMoveBitboard &= precomputedKingThreatBitboard; // Only include checking moves

        while (precomputedMoveBitboard) {
            uint8_t bitIndex = std::countr_zero(precomputedMoveBitboard); // Square on board
            moves.push_back(Move(currSquare, bitIndex));
            precomputedMoveBitboard &= precomputedMoveBitboard - 1; // Remove trailing set bit
        }
    }
}

void MoveGenerator::legalMoves(Board& board, Piece piece, Colour colour, 
                                uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalMoves(board, piece, colour, currSquare, moves);
    filterIllegalMoves(board, colour, moves);
}

void MoveGenerator::legalMoves(Board& board, Colour colour, std::vector<Move>& moves) {
    pseudoLegalMoves(board, colour, moves);
    filterIllegalMoves(board, colour, moves);
}

void MoveGenerator::legalCaptures(Board& board, Piece piece, Colour colour, 
                                    uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalCaptures(board, piece, colour, currSquare, moves);
    filterIllegalMoves(board, colour, moves);
}

void MoveGenerator::legalCaptures(Board& board, Colour colour, std::vector<Move>& moves) {
    constexpr Piece pieces[6] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, 
                                Piece::ROOK, Piece::QUEEN, Piece::KING};
    
    for (Piece piece : pieces) {
        Bitboard bitboard = board.getBitboard(piece, colour);
        while (bitboard) {
            uint8_t square = std::countr_zero(bitboard);
            pseudoLegalCaptures(board, piece, colour, square, moves);
            bitboard &= (bitboard - 1);
        }
    }

    filterIllegalMoves(board, colour, moves);
}

void MoveGenerator::filterIllegalMoves(Board& board, Colour colour, std::vector<Move>& moves) {
    auto castlingRightsBeforeMove = board.getCastlingRights();
    auto enPassantSquareBeforeMove = board.getEnPassantSquare();

    std::vector<Move> filtered;
    filtered.reserve(moves.capacity());
    for (Move move : moves) {
        board.makeMove(move, colour);
        bool inCheck = Check::isInCheck(board, colour);
        board.undo(move, colour, castlingRightsBeforeMove, enPassantSquareBeforeMove);

        if (!inCheck) filtered.push_back(move);
    }

    moves = std::move(filtered);
}

void MoveGenerator::pseudoLegalMoves(const Board& board, Colour colour, std::vector<Move>& moves) {
    constexpr Piece pieces[6] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, 
                                Piece::ROOK, Piece::QUEEN, Piece::KING};
    
    for (Piece piece : pieces) pseudoLegalMoves(board, piece, colour, moves);
}

void MoveGenerator::pseudoLegalMoves(const Board& board, Piece piece, Colour colour, std::vector<Move>& moves) {
    Bitboard bitboard = board.getBitboard(piece, colour);
    while (bitboard) {
        uint8_t square = std::countr_zero(bitboard);
        pseudoLegalMoves(board, piece, colour, square, moves);
        bitboard &= (bitboard - 1);
    }
}

void MoveGenerator::pseudoLegalMoves(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves) {
    assert(currSquare < 64 && "currSquare must be between 0-63");
    switch (piece) {
        case Piece::PAWN:
            pseudoLegalPawnMoves(board, colour, currSquare, moves);
            break;
        case Piece::KNIGHT:
            pseudoLegalKnightMoves(board, colour, currSquare, moves);
            break;
        case Piece::BISHOP:
            pseudoLegalBishopMoves(board, colour, currSquare, moves);
            break;
        case Piece::ROOK:
            pseudoLegalRookMoves(board, colour, currSquare, moves);
            break;
        case Piece::QUEEN:
            pseudoLegalQueenMoves(board, colour, currSquare, moves);
            break;
        case Piece::KING:
            pseudoLegalKingMoves(board, colour, currSquare, moves);
            break;
        default:
            assert(false && "Piece must be either PAWN, KNIGHT, BISHOP, ROOK, QUEEN or KING");
    }
}

void MoveGenerator::pseudoLegalCaptures(const Board& board, Colour colour, std::vector<Move>& moves) {
    constexpr Piece pieces[6] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, 
                                Piece::ROOK, Piece::QUEEN, Piece::KING};
    
    for (Piece piece : pieces) {
        Bitboard bitboard = board.getBitboard(piece, colour);
        while (bitboard) {
            uint8_t square = std::countr_zero(bitboard);
            pseudoLegalCaptures(board, piece, colour, square, moves);
            bitboard &= (bitboard - 1);
        }
    }
}

void MoveGenerator::pseudoLegalCaptures(const Board& board, Piece piece, Colour colour, uint8_t currSquare, std::vector<Move>& moves) {
    assert(currSquare < 64 && "currSquare must be between 0-63");
    switch (piece) {
        case Piece::PAWN:
            pseudoLegalPawnCaptures(board, colour, currSquare, moves);
            break;
        case Piece::KNIGHT:
            pseudoLegalKnightCaptures(board, colour, currSquare, moves);
            break;
        case Piece::BISHOP:
            pseudoLegalBishopCaptures(board, colour, currSquare, moves);
            break;
        case Piece::ROOK:
            pseudoLegalRookCaptures(board, colour, currSquare, moves);
            break;
        case Piece::QUEEN:
            pseudoLegalQueenCaptures(board, colour, currSquare, moves);
            break;
        case Piece::KING:
            pseudoLegalKingCaptures(board, colour, currSquare, moves);
            break;
        default:
            assert(false && "Piece must be either PAWN, KNIGHT, BISHOP, ROOK, QUEEN or KING");
    }
}

void MoveGenerator::pseudoLegalQueenPromotions(const Board& board, Colour colour, std::vector<Move>& moves) {
    uint8_t promotionPawnRanks[2] = {6, 1};
    uint8_t c = toIndex(colour);
    Bitboard pawnsBitboard = board.getBitboard(Piece::PAWN, colour);

    while (pawnsBitboard) {
        uint8_t square = std::countr_zero(pawnsBitboard);
        if (Board::getRank(square) == promotionPawnRanks[c]) {
            // 1 square forward promotion
            Bitboard singlePawnPush = PrecomputeMoves::singlePawnPushTable[c][square];
            uint8_t singlePawnPushSquare = std::countr_zero(singlePawnPush);
            bool singlePawnPushSquareEmpty = board.isEmpty(singlePawnPushSquare);

            if (singlePawnPushSquareEmpty) {
                moves.push_back(Move(square, singlePawnPushSquare, Move::NO_CAPTURE, toIndex(Piece::QUEEN)));
            }

            // Capture promotions
            Bitboard precomputedCaptures = PrecomputeMoves::pawnCaptureTable[c][square];
            Bitboard captureBitboard = precomputedCaptures & board.getOpposingBitboard(colour);
            while (captureBitboard) {
                uint8_t captureSquare = std::countr_zero(captureBitboard);
                uint8_t capture = toIndex(board.getPiece(captureSquare));
                
                moves.push_back(Move(square, captureSquare, capture, toIndex(Piece::QUEEN)));

                captureBitboard &= captureBitboard - 1;
            }
        }

        pawnsBitboard &= (pawnsBitboard - 1);
    }
}

void MoveGenerator::pseudoLegalNonCaptureChecks(const Board& board, Colour colour, uint8_t opponentKingSquare, std::vector<Move>& moves) {
    constexpr Piece pieces[5] = {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN};
    
    for (Piece piece : pieces) {
        Bitboard bitboard = board.getBitboard(piece, colour);
        while (bitboard) {
            uint8_t square = std::countr_zero(bitboard);
            switch (piece) {
                case Piece::PAWN:
                    pseudoLegalNonCapturePawnChecks(board, colour, square, opponentKingSquare, moves);
                    break;
                case Piece::KNIGHT:
                    pseudoLegalNonCaptureKnightChecks(board, colour, square, opponentKingSquare, moves);
                    break;
                case Piece::BISHOP:
                    pseudoLegalNonCaptureBishopChecks(board, colour, square, opponentKingSquare, moves);
                    break;
                case Piece::ROOK:
                    pseudoLegalNonCaptureRookChecks(board, colour, square, opponentKingSquare, moves);
                    break;
                case Piece::QUEEN:
                    pseudoLegalNonCaptureQueenChecks(board, colour, square, opponentKingSquare, moves);
                    break;
                default:
                    assert(false && "Piece must be either PAWN, KNIGHT, BISHOP, ROOK or QUEEN");
            }

            bitboard &= (bitboard - 1);
        }
    }
}

void MoveGenerator::pseudoLegalPawnMoves(const Board& board, Colour colour, 
                                        uint8_t currSquare, std::vector<Move>& moves) {

    uint8_t promotionPawnRanks[2] = {6, 1};
    uint8_t c = toIndex(colour);
    if (Board::getRank(currSquare) != promotionPawnRanks[c]) {
        Bitboard singlePawnPush = PrecomputeMoves::singlePawnPushTable[c][currSquare];
        uint8_t singlePawnPushSquare = std::countr_zero(singlePawnPush);
        bool singlePawnPushSquareEmpty = board.isEmpty(singlePawnPushSquare);

        if (singlePawnPushSquareEmpty) {
            moves.push_back(Move(currSquare, singlePawnPushSquare));
        }

        Bitboard doublePawnPush = PrecomputeMoves::doublePawnPushTable[c][currSquare];
        if (doublePawnPush) {
            uint8_t doublePawnPushSquare = std::countr_zero(doublePawnPush);
            bool doublePawnPushSquareEmpty = board.isEmpty(doublePawnPushSquare);
            
            if (singlePawnPushSquareEmpty && doublePawnPushSquareEmpty) {
                moves.push_back(Move(currSquare, doublePawnPushSquare));
            }
        }

    } else {
        // Possible promotion
        Bitboard singlePawnPush = PrecomputeMoves::singlePawnPushTable[c][currSquare];
        uint8_t singlePawnPushSquare = std::countr_zero(singlePawnPush);
        bool singlePawnPushSquareEmpty = board.isEmpty(singlePawnPushSquare);

        if (singlePawnPushSquareEmpty) {
            moves.push_back(Move(currSquare, singlePawnPushSquare, Move::NO_CAPTURE, toIndex(Piece::KNIGHT)));
            moves.push_back(Move(currSquare, singlePawnPushSquare, Move::NO_CAPTURE, toIndex(Piece::BISHOP)));
            moves.push_back(Move(currSquare, singlePawnPushSquare, Move::NO_CAPTURE, toIndex(Piece::ROOK)));
            moves.push_back(Move(currSquare, singlePawnPushSquare, Move::NO_CAPTURE, toIndex(Piece::QUEEN)));
        }
    }

    pseudoLegalPawnCaptures(board, colour, currSquare, moves);
}

void MoveGenerator::pseudoLegalKnightMoves(const Board& board, Colour colour, 
                                            uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalMovesFromTable(board, colour, currSquare, moves, PrecomputeMoves::knightMoveTable[currSquare]);
}

void MoveGenerator::pseudoLegalBishopMoves(const Board& board, Colour colour, 
                                            uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalMovesFromTable(board, colour, currSquare, moves, PrecomputeMoves::getBishopMovesFromTable(currSquare, board.getPiecesBitboard()));
}

void MoveGenerator::pseudoLegalRookMoves(const Board& board, Colour colour, 
                                        uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalMovesFromTable(board, colour, currSquare, moves, PrecomputeMoves::getRookMovesFromTable(currSquare, board.getPiecesBitboard()));
}

void MoveGenerator::pseudoLegalQueenMoves(const Board& board, Colour colour, 
                                        uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalBishopMoves(board, colour, currSquare, moves);
    pseudoLegalRookMoves(board, colour, currSquare, moves);
}

void MoveGenerator::pseudoLegalKingMoves(const Board& board, Colour colour, 
                                        uint8_t currSquare, std::vector<Move>& moves) {

    // Regular moves
    pseudoLegalMovesFromTable(board, colour, currSquare, moves, PrecomputeMoves::kingMoveTable[currSquare]);

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

void MoveGenerator::pseudoLegalPawnCaptures(const Board& board, Colour colour, 
                                            uint8_t currSquare, std::vector<Move>& moves) {

    uint8_t promotionPawnRanks[2] = {6, 1};
    uint8_t c = toIndex(colour);

    if (Board::getRank(currSquare) != promotionPawnRanks[c]) {
        // Diagonal captures
        Bitboard precomputedCaptures = PrecomputeMoves::pawnCaptureTable[c][currSquare];
        Bitboard captureBitboard = precomputedCaptures & board.getOpposingBitboard(colour);
        while (captureBitboard) {
            uint8_t captureSquare = std::countr_zero(captureBitboard);
            uint8_t capture = toIndex(board.getPiece(captureSquare));
            moves.push_back(Move(currSquare, captureSquare, capture));

            captureBitboard &= captureBitboard - 1;
        }

        // En passant
        const std::optional<uint8_t> enPassantSquare = board.getEnPassantSquare();
        if (enPassantSquare.has_value()) {
            Bitboard enPassantAttacks = PrecomputeMoves::enPassantSquareTable[c][currSquare];
            int directions[2] = {1, -1};
            while (enPassantAttacks) {
                uint8_t enPassantAttackSquare = std::countr_zero(enPassantAttacks);
                if (enPassantAttackSquare == *enPassantSquare) {
                    uint8_t captureSquare = enPassantAttackSquare + 8 * directions[c];
                    moves.push_back(Move(currSquare, captureSquare, toIndex(Piece::PAWN), Move::NO_PROMOTION, Move::NO_CASTLE, 1));
                }

                enPassantAttacks &= enPassantAttacks - 1;
            }
        }

    } else {
        // Possible promotion captures
        Bitboard precomputedCaptures = PrecomputeMoves::pawnCaptureTable[c][currSquare];
        Bitboard captureBitboard = precomputedCaptures & board.getOpposingBitboard(colour);
        while (captureBitboard) {
            uint8_t captureSquare = std::countr_zero(captureBitboard);
            uint8_t capture = toIndex(board.getPiece(captureSquare));
            
            moves.push_back(Move(currSquare, captureSquare, capture, toIndex(Piece::KNIGHT)));
            moves.push_back(Move(currSquare, captureSquare, capture, toIndex(Piece::BISHOP)));
            moves.push_back(Move(currSquare, captureSquare, capture, toIndex(Piece::ROOK)));
            moves.push_back(Move(currSquare, captureSquare, capture, toIndex(Piece::QUEEN)));

            captureBitboard &= captureBitboard - 1;
        }
    }
}

void MoveGenerator::pseudoLegalKnightCaptures(const Board& board, Colour colour, 
                                            uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalCapturesFromTable(board, colour, currSquare, moves, PrecomputeMoves::knightMoveTable[currSquare]);
}

void MoveGenerator::pseudoLegalBishopCaptures(const Board& board, Colour colour, 
                                            uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalCapturesFromTable(board, colour, currSquare, moves, PrecomputeMoves::getBishopMovesFromTable(currSquare, board.getPiecesBitboard()));
}

void MoveGenerator::pseudoLegalRookCaptures(const Board& board, Colour colour, 
                                            uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalCapturesFromTable(board, colour, currSquare, moves, PrecomputeMoves::getRookMovesFromTable(currSquare, board.getPiecesBitboard()));
}

void MoveGenerator::pseudoLegalQueenCaptures(const Board& board, Colour colour, 
                                            uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalBishopCaptures(board, colour, currSquare, moves);
    pseudoLegalRookCaptures(board, colour, currSquare, moves);
}

void MoveGenerator::pseudoLegalKingCaptures(const Board& board, Colour colour, 
                                            uint8_t currSquare, std::vector<Move>& moves) {

    pseudoLegalCapturesFromTable(board, colour, currSquare, moves, PrecomputeMoves::kingMoveTable[currSquare]);
}

void MoveGenerator::pseudoLegalNonCapturePawnChecks(const Board& board, Colour colour, uint8_t currSquare, uint8_t opponentKingSquare, std::vector<Move>& moves) {
    uint8_t c = toIndex(colour);
    Bitboard movesBitboard = 0ULL;

    Bitboard singlePawnPush = PrecomputeMoves::singlePawnPushTable[c][currSquare];
    uint8_t singlePawnPushSquare = std::countr_zero(singlePawnPush);
    bool singlePawnPushSquareEmpty = board.isEmpty(singlePawnPushSquare);

    if (singlePawnPushSquareEmpty) {
        movesBitboard = singlePawnPush;
    }

    Bitboard doublePawnPush = PrecomputeMoves::doublePawnPushTable[c][currSquare];
    if (doublePawnPush) {
        uint8_t doublePawnPushSquare = std::countr_zero(doublePawnPush);
        bool doublePawnPushSquareEmpty = board.isEmpty(doublePawnPushSquare);
        
        if (singlePawnPushSquareEmpty && doublePawnPushSquareEmpty) {
            movesBitboard |= doublePawnPush;
        }
    }

    pseudoLegalNonCaptureChecksFromTable(board, colour, currSquare, moves, movesBitboard, PrecomputeMoves::pawnThreatTable[c][opponentKingSquare]);
}

void MoveGenerator::pseudoLegalNonCaptureKnightChecks(const Board& board, Colour colour, uint8_t currSquare, uint8_t opponentKingSquare, std::vector<Move>& moves) {
    pseudoLegalNonCaptureChecksFromTable(board, colour, currSquare, moves, 
                                        PrecomputeMoves::knightMoveTable[currSquare], 
                                        PrecomputeMoves::knightMoveTable[opponentKingSquare]);
}

void MoveGenerator::pseudoLegalNonCaptureBishopChecks(const Board& board, Colour colour, uint8_t currSquare, uint8_t opponentKingSquare, std::vector<Move>& moves) {
    pseudoLegalNonCaptureChecksFromTable(board, colour, currSquare, moves, 
                                        PrecomputeMoves::getBishopMovesFromTable(currSquare, board.getPiecesBitboard()), 
                                        PrecomputeMoves::getBishopMovesFromTable(opponentKingSquare, board.getPiecesBitboard()));
}

void MoveGenerator::pseudoLegalNonCaptureRookChecks(const Board& board, Colour colour, uint8_t currSquare, uint8_t opponentKingSquare, std::vector<Move>& moves) {
    pseudoLegalNonCaptureChecksFromTable(board, colour, currSquare, moves, 
                                        PrecomputeMoves::getRookMovesFromTable(currSquare, board.getPiecesBitboard()), 
                                        PrecomputeMoves::getRookMovesFromTable(opponentKingSquare, board.getPiecesBitboard()));
}

void MoveGenerator::pseudoLegalNonCaptureQueenChecks(const Board& board, Colour colour, uint8_t currSquare, uint8_t opponentKingSquare, std::vector<Move>& moves) {
    pseudoLegalNonCaptureBishopChecks(board, colour, currSquare, opponentKingSquare, moves);
    pseudoLegalNonCaptureRookChecks(board, colour, currSquare, opponentKingSquare, moves);
}