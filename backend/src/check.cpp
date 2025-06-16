#include <cstdint>
#include "board.h"
#include "check.h"
#include "chess_types.h"
#include "precompute_moves.h"

using Bitboard = Chess::Bitboard;
using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;

GameState Check::evaluateGameState(const Board& board, Colour colour) {
    bool isCheck = isInCheck(board, colour);
    bool hasLegalMove = hasMove(board, colour);

    if (isCheck && !hasLegalMove) return GameState::CHECKMATE;
    if (!hasLegalMove) return GameState::STALEMATE;
    if (isCheck) return GameState::CHECK;

    return GameState::IN_PROGRESS;
}

bool Check::isInCheck(const Board& board, Colour colour) {
    uint8_t kingSquare = board.getKingSquare(colour);

    // Knights
    Bitboard knightAttacksBitboard = PrecomputeMoves::knightMoveTable[kingSquare];
    Bitboard opposingKnightsBitboard = board.getOpposingBitboard(Piece::KNIGHT, colour);
    if (knightAttacksBitboard & opposingKnightsBitboard) return true;

    // Pawns
    Bitboard pawnAttacksBitboard = (colour == Colour::WHITE) ?
                                    PrecomputeMoves::blackPawnCaptureTable[kingSquare] :
                                    PrecomputeMoves::whitePawnCaptureTable[kingSquare];

    Bitboard opposingPawnsBitboard = board.getOpposingBitboard(Piece::PAWN, colour);
    if (pawnAttacksBitboard & opposingPawnsBitboard) return true;

    // King
    Bitboard kingAttacksBitboard = PrecomputeMoves::kingMoveTable[kingSquare];
    Bitboard opposingKingBitboard = board.getOpposingBitboard(Piece::KING, colour);
    if (kingAttacksBitboard & opposingKingBitboard) return true;

    // Bishops, Rooks & Queens
    using Function = uint8_t(*)(uint8_t);
    constexpr int orthogonalDirections[4] = {-1, 8, 1, -8}; // ←, ↑, →, ↓
    constexpr int diagonalDirections[4] = {7, 9, -9, -7}; // ↖, ↗, ↙, ↘
    constexpr uint8_t boundaryChecks[4] = {0, 7, 7, 0}; // file, rank, file, rank
    constexpr uint8_t rankChecks[4] = {7, 7, 0, 0}; // Boundaries depending on direction for diagonals
    constexpr uint8_t fileChecks[4] = {0, 7, 0, 7}; // Boundaries depending on direction for diagonals
    Function functions[2] = {Board::getFile, Board::getRank}; // Alternate between file and rank checks

    for (int i = 0; i < 4; i++) {
        // Orthogonal directions
        if (functions[i & 0x1](kingSquare) != boundaryChecks[i]) { // Toggle between file and rank checks at each iteration
            uint8_t square = kingSquare + orthogonalDirections[i];
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
        if (Board::getFile(kingSquare) != fileChecks[i] && Board::getRank(kingSquare) != rankChecks[i]) {
            uint8_t square = kingSquare + diagonalDirections[i];
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

bool Check::hasMove(const Board& board, Colour colour) {

}