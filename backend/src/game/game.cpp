#include <stack>
#include <vector>
#include <optional>
#include <cstdint>
#include <bit>
#include "game/game.h"
#include "game/game_state.h"
#include "board/board.h"
#include "move/move.h"
#include "move/move_info.h"
#include "move/move_generator.h"
#include "check/check.h"
#include "zobrist_hash.h"
#include "chess_types.h"

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;
using Chess::Bitboard;
using Chess::toIndex;

namespace {
    std::optional<Move> searchLegalMoves(const std::vector<Move>& legalMoves, 
                                            uint8_t fromSquare, 
                                            uint8_t toSquare, 
                                            std::optional<uint8_t> promotion = std::nullopt) {

        for (const Move& move : legalMoves) {
            if (move.getFromSquare() == fromSquare && 
                move.getToSquare() == toSquare && 
                (!promotion.has_value() || move.getPromotionPiece() == *promotion)) {

                return move;
            }
        }

        return std::nullopt;
    }
}

namespace {
    bool containsOnePiece(const Bitboard bitboard) {
        return (bitboard != 0ULL && (bitboard & (bitboard - 1)) == 0ULL);
    }

    bool insufficientMaterialHelper(const Board& board, const Colour colour1, const Colour colour2) {
        if (containsOnePiece(board.getBitboard(colour1))) {
            // King vs King + Bishop
            if (board.getBitboard(Piece::KNIGHT, colour2) == 0ULL &&
                containsOnePiece(board.getBitboard(Piece::BISHOP, colour2))) {
                    return true;
                }
            // King vs King + Knight
            if (board.getBitboard(Piece::BISHOP, colour2) == 0ULL &&
                containsOnePiece(board.getBitboard(Piece::KNIGHT, colour2))) {
                    return true;
                }
        }

        return false;
    }

    bool isSameColourBishopSquares(Bitboard bitboard1, Bitboard bitboard2) {
        uint8_t square1 = std::countr_zero(bitboard1);
        uint8_t square2 = std::countr_zero(bitboard2);

        uint8_t rank1 = Board::getRank(square1), file1 = Board::getFile(square1);
        uint8_t rank2 = Board::getRank(square2), file2 = Board::getFile(square2);

        return (((rank1 + file1) & 0x1) == ((rank2 + file2) & 0x1));
    }
}

Game::Game() : currentTurn(Colour::WHITE) {
    uint64_t hash = Zobrist::computeInitialHash(board, currentTurn);
    gameStateHistory.push(createGameState(currentTurn, board.getEnPassantSquare(), board.getCastlingRights(), 0, 1, hash));
}

GameStateEvaluation Game::getCurrentGameStateEvaluation() {
    if (isDrawByFiftyMoveRule()) return GameStateEvaluation::DRAW_BY_FIFTY_MOVE_RULE;
    if (isDrawByInsufficientMaterial()) return GameStateEvaluation::DRAW_BY_INSUFFICIENT_MATERIAL;

    CheckEvaluation checkEvaluation = Check::evaluateGameState(board, currentTurn);
    switch (checkEvaluation) {
        case CheckEvaluation::CHECKMATE: return GameStateEvaluation::CHECKMATE;
        case CheckEvaluation::STALEMATE: return GameStateEvaluation::STALEMATE;
        case CheckEvaluation::CHECK: return GameStateEvaluation::CHECK;
        case CheckEvaluation::NONE: return GameStateEvaluation::IN_PROGRESS;
    }
}

Colour Game::getCurrentTurn() {
    return currentTurn;
}

bool Game::makeMove(uint8_t fromSquare, uint8_t toSquare, uint8_t promotion) {
    auto [pieceOpt, colourOpt] = board.getPieceAndColour(fromSquare);
    if (!pieceOpt.has_value() || !colourOpt.has_value()) return false;

    Piece piece = *pieceOpt;
    Colour colour = *colourOpt;

    std::vector<Move> legalMoves = MoveGenerator::legalMoves(board, piece, colour, fromSquare);
    std::optional<Move> moveOpt = searchLegalMoves(legalMoves, fromSquare, toSquare, promotion);

    if (!moveOpt.has_value()) return false;

    Move move = *moveOpt;
    board.makeMove(move, colour);
    Colour newPlayerTurn = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    uint16_t newFullMoves = (colour == Colour::BLACK) ? currentState.fullMoves + 1 : currentState.fullMoves;
    uint16_t newHalfMoves = (move.getCapturedPiece() == Move::NO_CAPTURE && piece != Piece::PAWN) ?
                            currentState.halfMoveClock + 1 :
                            0;

    auto newEnPassantSquare = board.getEnPassantSquare();
    auto newCastlingRights = board.getCastlingRights();
    uint64_t newHash = Zobrist::updateHash(currentState.hash, move, currentState.enPassantSquare, newEnPassantSquare,
                                            currentState.castleRights, newCastlingRights, colour, piece);

    currentState = createGameState(newPlayerTurn, newEnPassantSquare, newCastlingRights, newHalfMoves, newFullMoves, newHash);
    moveHistory.push(move);
    gameStateHistory.push(currentState);
    currentTurn = newPlayerTurn;
    return true;
}

bool Game::undo() {
    if (gameStateHistory.size() == 1) return false;

    const Move& previousMove = moveHistory.top();
    const GameState& previousState = gameStateHistory.top();
    board.undo(previousMove, previousState.playerTurn, previousState.castleRights, previousState.enPassantSquare);

    currentState = previousState;
    gameStateHistory.pop();
    moveHistory.pop();
    currentTurn = (currentTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;

    return true;    
}

bool Game::isCurrentPlayerOccupies(uint8_t square) {
    std::optional<Colour> colourOpt = board.getColour(square);
    if (!colourOpt.has_value()) return false;

    Colour colour = *colourOpt;
    return colour == currentTurn;
}

std::vector<Move> Game::getLegalMoves(uint8_t square) {
    auto [pieceOpt, colourOpt] = board.getPieceAndColour(square);
    if (!pieceOpt.has_value() || !colourOpt.has_value()) return {};

    return MoveGenerator::legalMoves(board, *pieceOpt, *colourOpt, square);
}

std::optional<MoveInfo> Game::getMoveInfo(uint8_t fromSquare, uint8_t toSquare) {
    auto [pieceOpt, colourOpt] = board.getPieceAndColour(fromSquare);
    if (!pieceOpt.has_value() || !colourOpt.has_value()) return std::nullopt;

    Piece piece = *pieceOpt;
    Colour colour = *colourOpt;

    std::vector<Move> legalMoves = MoveGenerator::legalMoves(board, piece, colour, fromSquare);
    std::optional<Move> moveOpt = searchLegalMoves(legalMoves, fromSquare, toSquare);

    if (!moveOpt) return std::nullopt;

    Move move = *moveOpt;
    auto [capturedPieceOpt, capturedColourOpt] = board.getPieceAndColour(toSquare);
    uint8_t capturedPiece = (capturedPieceOpt.has_value()) ? 
                            toIndex(*capturedPieceOpt) :
                            Move::NO_CAPTURE;
    uint8_t capturedColour = (capturedColourOpt.has_value()) ? 
                            toIndex(*capturedColourOpt) :
                            Move::NO_CAPTURE;

    return std::optional<MoveInfo>({move, toIndex(piece), toIndex(colour), capturedPiece, capturedColour});
}

bool Game::isDrawByFiftyMoveRule() {
    return (currentState.halfMoveClock == 100);
}

bool Game::isDrawByRepetition() {

}

bool Game::isDrawByInsufficientMaterial() {
    if (board.getBitboard(Piece::PAWN, Colour::WHITE) != 0ULL || 
        board.getBitboard(Piece::PAWN, Colour::BLACK) != 0ULL ||
        board.getBitboard(Piece::ROOK, Colour::WHITE) != 0ULL || 
        board.getBitboard(Piece::ROOK, Colour::BLACK) != 0ULL ||
        board.getBitboard(Piece::QUEEN, Colour::WHITE) != 0ULL || 
        board.getBitboard(Piece::QUEEN, Colour::BLACK) != 0ULL) {
            
        return false;
    }

    // King vs King
    if (containsOnePiece(board.getBitboard(Colour::WHITE)) && 
        containsOnePiece(board.getBitboard(Colour::BLACK))) {

        return true;
    }

    // King + Bishop vs King + Bishop with same colour squared bishops
    Bitboard whiteBishop = board.getBitboard(Piece::BISHOP, Colour::WHITE);
    Bitboard blackBishop = board.getBitboard(Piece::BISHOP, Colour::BLACK);
    if (board.getBitboard(Piece::KNIGHT, Colour::WHITE) == 0ULL &&
        board.getBitboard(Piece::KNIGHT, Colour::BLACK) == 0ULL &&
        containsOnePiece(whiteBishop) &&
        containsOnePiece(blackBishop) &&
        isSameColourBishopSquares(whiteBishop, blackBishop)) {

        return true;
    }

    return insufficientMaterialHelper(board, Colour::WHITE, Colour::BLACK) || 
           insufficientMaterialHelper(board, Colour::BLACK, Colour::WHITE);
}