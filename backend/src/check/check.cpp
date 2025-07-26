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
    // Knights
    Bitboard knightAttacksBitboard = PrecomputeMoves::knightMoveTable[targetSquare];
    Bitboard opposingKnightsBitboard = board.getOpposingBitboard(Piece::KNIGHT, colour);
    if (knightAttacksBitboard & opposingKnightsBitboard) return true;

    // Pawns
    Bitboard pawnAttacksBitboard = (colour == Colour::WHITE) ?
                                    PrecomputeMoves::blackPawnCaptureTable[targetSquare] :
                                    PrecomputeMoves::whitePawnCaptureTable[targetSquare];

    Bitboard opposingPawnsBitboard = board.getOpposingBitboard(Piece::PAWN, colour);
    if (pawnAttacksBitboard & opposingPawnsBitboard) return true;

    // King
    Bitboard kingAttacksBitboard = PrecomputeMoves::kingMoveTable[targetSquare];
    Bitboard opposingKingBitboard = board.getOpposingBitboard(Piece::KING, colour);
    if (kingAttacksBitboard & opposingKingBitboard) return true;

    // Bishops, Rooks & Queens
    using Function = uint8_t(*)(uint8_t);
    constexpr int orthogonalDirections[4] = {-1, 8, 1, -8}; // ←, ↑, →, ↓
    constexpr int diagonalDirections[4] = {7, 9, -9, -7}; // ↖, ↗, ↙, ↘
    constexpr uint8_t boundaryChecks[4] = {0, 7, 7, 0}; // file, rank, file, rank
    constexpr uint8_t rankChecks[4] = {7, 7, 0, 0}; // Boundaries depending on direction for diagonals
    constexpr uint8_t fileChecks[4] = {0, 7, 0, 7}; // Boundaries depending on direction for diagonals
    constexpr Function functions[2] = {Board::getFile, Board::getRank}; // Alternate between file and rank checks

    for (int i = 0; i < 4; i++) {
        // Orthogonal directions
        if (functions[i & 0x1](targetSquare) != boundaryChecks[i]) { // Toggle between file and rank checks at each iteration
            uint8_t square = targetSquare + orthogonalDirections[i];
            // Not at edge of the board and square is empty
            while (functions[i & 0x1](square) != boundaryChecks[i] && board.isEmpty(square)) {
                square += orthogonalDirections[i];
            }
            // Final square is either an empty square on the edge of the board or an occupied square
            if (board.isOccupied(square) && (board.isOpponentOccupied(Piece::QUEEN, colour, square) ||
                                             board.isOpponentOccupied(Piece::ROOK, colour, square))) {
                
                return true;
            }
        }

        // Diagonal directions
        if (Board::getFile(targetSquare) != fileChecks[i] && Board::getRank(targetSquare) != rankChecks[i]) {
            uint8_t square = targetSquare + diagonalDirections[i];
            // Not at edge of the board and square is empty
            while (Board::getFile(square) != fileChecks[i] && Board::getRank(square) != rankChecks[i] && board.isEmpty(square)) {
                square += diagonalDirections[i];
            }
            // Final square is either an empty square on the edge of the board or an occupied square
            if (board.isOccupied(square) && (board.isOpponentOccupied(Piece::QUEEN, colour, square) ||
                                             board.isOpponentOccupied(Piece::BISHOP, colour, square))) {
                
                return true;
            }
        }
    }

    return false;
}

bool Check::isInCheck(const Board& board, Colour colour) {
    uint8_t kingSquare = board.getKingSquare(colour);
    return isInDanger(board, colour, kingSquare);
}

bool Check::hasMove(Board& board, Colour colour) {
    moveBuffer.clear();
    MoveGenerator::legalMoves(board, colour, moveBuffer);
    return !moveBuffer.empty();
}