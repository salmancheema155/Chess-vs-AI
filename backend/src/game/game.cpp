#include <stack>
#include <vector>
#include <optional>
#include <cstdint>
#include "game/game.h"
#include "game/game_state.h"
#include "board/board.h"
#include "move/move.h"
#include "move/move_generator.h"
#include "zobrist_hash.h"
#include "chess_types.h"

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;

namespace {
    std::optional<Move> searchLegalMoves(const std::vector<Move>& legalMoves, uint8_t fromSquare, uint8_t toSquare) {
        for (const Move& move : legalMoves) {
            if (move.getFromSquare() == fromSquare && move.getToSquare() == toSquare) {
                return std::optional<Move>(move);
            }
        }
        return std::nullopt;
    }
}

Game::Game() : currentTurn(Colour::WHITE) {
    uint64_t hash = Zobrist::computeInitialHash(board, currentTurn);
    gameStateHistory.push(createGameState(currentTurn, board.getEnPassantSquare(), board.getCastlingRights(), 0, 1, hash));
}

bool Game::makeMove(uint8_t fromSquare, uint8_t toSquare) {
    auto [pieceOpt, colourOpt] = board.getPieceAndColour(fromSquare);
    if (!pieceOpt.has_value() || !colourOpt.has_value()) return false;

    Piece piece = *pieceOpt;
    Colour colour = *colourOpt;

    std::vector<Move> legalMoves = MoveGenerator::legalMoves(board, piece, colour, fromSquare);
    std::optional<Move> moveOpt = searchLegalMoves(legalMoves, fromSquare, toSquare);

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