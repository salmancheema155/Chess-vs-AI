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

                return std::optional<Move>(move);
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
    positionHistory[hash] = 1;
    GameState currentState = createGameState(currentTurn, board.getEnPassantSquare(), board.getCastlingRights(), 0, 1, hash);
    gameStateHistory.push(currentState);
}

GameStateEvaluation Game::getCurrentGameStateEvaluation() {
    if (isDrawByRepetition()) return GameStateEvaluation::DRAW_BY_REPETITION;
    if (isDrawByFiftyMoveRule()) return GameStateEvaluation::DRAW_BY_FIFTY_MOVE_RULE;
    if (isDrawByInsufficientMaterial()) return GameStateEvaluation::DRAW_BY_INSUFFICIENT_MATERIAL;

    CheckEvaluation checkEvaluation = Check::evaluateGameState(board, currentTurn);
    switch (checkEvaluation) {
        case CheckEvaluation::CHECKMATE: return GameStateEvaluation::CHECKMATE;
        case CheckEvaluation::STALEMATE: return GameStateEvaluation::STALEMATE;
        case CheckEvaluation::CHECK: return GameStateEvaluation::CHECK;
        case CheckEvaluation::NONE: return GameStateEvaluation::IN_PROGRESS;
    }

    return GameStateEvaluation::IN_PROGRESS;
}

Colour Game::getColour(uint8_t square) {
    return board.getColour(square);
}

void Game::makeMove(const Move& move) {
    auto [piece, colour] = board.getPieceAndColour(move.getFromSquare());
    board.makeMove(move, colour);
    GameState& currentState = gameStateHistory.top();
    Colour newPlayerTurn = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    uint16_t newFullMoves = (colour == Colour::BLACK) ? currentState.fullMoves + 1 : currentState.fullMoves;
    uint8_t newHalfMoves = (move.getCapturedPiece() == Move::NO_CAPTURE && piece != Piece::PAWN) ?
                            currentState.halfMoveClock + 1 :
                            0;

    auto newEnPassantSquare = board.getEnPassantSquare();
    auto newCastlingRights = board.getCastlingRights();
    uint64_t newHash = Zobrist::updateHash(currentState.hash, move, currentState.enPassantSquare, newEnPassantSquare,
                                            currentState.castleRights, newCastlingRights, colour, piece);

    GameState newState = createGameState(newPlayerTurn, newEnPassantSquare, newCastlingRights, newHalfMoves, newFullMoves, newHash);
    positionHistory[newHash]++;
    moveHistory.push(move);
    gameStateHistory.push(newState);
    currentTurn = newPlayerTurn;
}

bool Game::makeMove(uint8_t fromSquare, uint8_t toSquare, uint8_t promotion) {
    auto [piece, colour] = board.getPieceAndColour(fromSquare);
    if (piece == Piece::NONE || colour == Colour::NONE) return false;

    std::vector<Move> legalMoves = MoveGenerator::legalMoves(board, piece, colour, fromSquare);
    std::optional<Move> moveOpt = searchLegalMoves(legalMoves, fromSquare, toSquare, std::optional<uint8_t>(promotion));

    if (!moveOpt.has_value()) return false;

    Move move = *moveOpt;
    this->makeMove(move);

    return true;
}

bool Game::undo() {       
    if (gameStateHistory.size() == 1 || moveHistory.size() == 0) return false;

    undoHash(gameStateHistory.top().hash);
    gameStateHistory.pop();
    const Move& previousMove = moveHistory.top();
    const GameState& previousState = gameStateHistory.top();
    board.undo(previousMove, previousState.playerTurn, previousState.castleRights, previousState.enPassantSquare);

    currentTurn = previousState.playerTurn;
    moveHistory.pop();

    return true;    
}

void Game::undoHash(uint64_t hash) {
    if (positionHistory[hash] == 0) positionHistory.erase(hash);
    else positionHistory[hash]--;
}

bool Game::isCurrentPlayerOccupies(uint8_t square) {
    Colour colour = board.getColour(square);
    if (colour == Colour::NONE) return false;

    return colour == currentTurn;
}

std::vector<Move> Game::getLegalMoves(uint8_t square) {
    auto [piece, colour] = board.getPieceAndColour(square);
    if (piece == Piece::NONE || colour == Colour::NONE) return {};

    return MoveGenerator::legalMoves(board, piece, colour, square);
}

std::optional<MoveInfo> Game::getMoveInfo(uint8_t fromSquare, uint8_t toSquare, uint8_t promotion) {
    auto [piece, colour] = board.getPieceAndColour(fromSquare);
    if (piece == Piece::NONE || colour == Colour::NONE) return std::nullopt;

    std::vector<Move> legalMoves = MoveGenerator::legalMoves(board, piece, colour, fromSquare);
    std::optional<Move> moveOpt = searchLegalMoves(legalMoves, fromSquare, toSquare, std::optional<uint8_t>(promotion));

    if (!moveOpt) return std::nullopt;

    Move move = *moveOpt;
    auto [capturedPiece, capturedColour] = board.getPieceAndColour(toSquare);

    return std::optional<MoveInfo>({move, toIndex(piece), toIndex(colour), toIndex(capturedPiece), toIndex(capturedColour)});
}

bool Game::isPromotionMove(uint8_t fromSquare, uint8_t toSquare) {
    auto [piece, colour] = board.getPieceAndColour(fromSquare);
    if (piece != Piece::PAWN || colour == Colour::NONE) return false;

    std::vector<Move> legalMoves = MoveGenerator::legalMoves(board, piece, colour, fromSquare);
    std::optional<Move> moveOpt = searchLegalMoves(legalMoves, fromSquare, toSquare);

    if (!moveOpt) return false;

    Move move = *moveOpt;
    return (move.getPromotionPiece() != Move::NO_PROMOTION);
}

bool Game::isDrawByFiftyMoveRule() {
    return (gameStateHistory.top().halfMoveClock >= 100);
}

bool Game::isDrawByRepetition() {
    return (positionHistory[gameStateHistory.top().hash] >= 3);
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