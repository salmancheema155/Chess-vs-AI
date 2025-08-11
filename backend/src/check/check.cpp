#include <cstdint>
#include <vector>
#include "board/board.h"
#include "check/check.h"
#include "move/precompute_moves.h"
#include "move/move_generator.h"
#include "chess_types.h"

using Bitboard = Chess::Bitboard;
using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;

CheckEvaluation Check::evaluateGameState(Board& board, Colour colour) {
    bool isCheck = isInCheck(board, colour);
    bool hasLegalMove = hasMove(board, colour);

    if (isCheck && !hasLegalMove) return CheckEvaluation::CHECKMATE;
    if (!hasLegalMove) return CheckEvaluation::STALEMATE;
    if (isCheck) return CheckEvaluation::CHECK;

    return CheckEvaluation::NONE;
}

bool Check::isInDanger(const Board& board, Colour colour, uint8_t targetSquare) {
    Colour opposingColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    Bitboard occupiedBitboard = board.getPiecesBitboard();

    // Rooks/Queens (orthogonal)
    Bitboard rookAttacksBitboard = PrecomputeMoves::getRookMovesFromTable(targetSquare, occupiedBitboard);
    Bitboard opposingRooksQueensBitboard = board.getBitboard(Piece::ROOK, opposingColour) |
                                           board.getBitboard(Piece::QUEEN, opposingColour);
    if (rookAttacksBitboard & opposingRooksQueensBitboard) return true;

    // Bishops/Queens (diagonal)
    Bitboard bishopAttacksBitboard = PrecomputeMoves::getBishopMovesFromTable(targetSquare, occupiedBitboard);
    Bitboard opposingBishopsQueensBitboard = board.getBitboard(Piece::BISHOP, opposingColour) |
                                             board.getBitboard(Piece::QUEEN, opposingColour);
    if (bishopAttacksBitboard & opposingBishopsQueensBitboard) return true;

    // Knights
    Bitboard knightAttacksBitboard = PrecomputeMoves::knightMoveTable[targetSquare];
    Bitboard opposingKnightsBitboard = board.getBitboard(Piece::KNIGHT, opposingColour);
    if (knightAttacksBitboard & opposingKnightsBitboard) return true;

    // Pawns
    Bitboard pawnAttacksBitboard = PrecomputeMoves::pawnThreatTable[toIndex(opposingColour)][targetSquare];
    Bitboard opposingPawnsBitboard = board.getBitboard(Piece::PAWN, opposingColour);
    if (pawnAttacksBitboard & opposingPawnsBitboard) return true;

    // King
    Bitboard kingAttacksBitboard = PrecomputeMoves::kingMoveTable[targetSquare];
    Bitboard opposingKingBitboard = board.getBitboard(Piece::KING, opposingColour);
    if (kingAttacksBitboard & opposingKingBitboard) return true;

    return false;
}

bool Check::isInCheck(const Board& board, Colour colour) {
    uint8_t kingSquare = board.getKingSquare(colour);
    return isInDanger(board, colour, kingSquare);
}

bool Check::hasMove(Board& board, Colour colour) {
    constexpr Piece pieces[6] = {Piece::KING, Piece::KNIGHT, Piece::PAWN, 
                                Piece::BISHOP, Piece::ROOK, Piece::QUEEN};

    for (Piece piece : pieces) {
        moveBuffer.clear();
        MoveGenerator::pseudoLegalMoves(board, piece, colour, moveBuffer);

        auto castlingRightsBeforeMove = board.getCastlingRights();
        auto enPassantSquareBeforeMove = board.getEnPassantSquare();

        for (Move move : moveBuffer) {
            board.makeMove(move, colour);
            bool inCheck = Check::isInCheck(board, colour);
            board.undo(move, colour, castlingRightsBeforeMove, enPassantSquareBeforeMove);

            if (!inCheck) return true;
        }
    }

    return false;
}