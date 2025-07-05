#include <stack>
#include <vector>
#include <optional>
#include <cstdint>
#include "game/game.h"
#include "game/game_state.h"
#include "board/board.h"
#include "move/move_generator.h"
#include "zobrist_hash.h"
#include "chess_types.h"

using Piece = Chess::PieceColour;
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

Game::Game() {
    Colour currentTurn = Colour::WHITE;
    uint64_t hash = Zobrist::computeInitialHash(board, currentTurn);
    gameStateHistory.push(createGameState(currentTurn, board.getEnPassantSquare(), board.getCastlingRights(), 0, 1, hash));
}

bool Game::makeMove(uint8_t fromSquare, uint8_t toSquare) {
    auto [piece, colour] = board.getPieceAndColour(fromSquare);
    if (!piece.has_value() || !colour.has_value()) return false;

    std::vector<Move> legalMoves = MoveGenerator::legalMoves(board, *piece, *colour, fromSquare);
    std::optional<Move> move = searchLegalMoves(legalMoves, fromSquare, toSquare);

    if (!move.has_value()) return false;

    board.makeMove(*move, *colour);
    Colour newPlayerTurn = (*colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    uint16_t newFullMoves = (*colour == Colour::BLACK) ? currentState.fullMoves + 1 : currentState.fullMoves;
    uint16_t newHalfMoves = ((*move).getCapturedPiece() == Move::NO_CAPTURE && *piece != Piece::PAWN) ?
                            currentState.halfMoveClock + 1 :
                            0;

    auto newEnPassantSquare = board.getEnPassantSquare();
    auto newCastlingRights = board.getCastlingRights();
    uint64_t newHash = Zobrist::updateHash(currentState.hash, *move, currentState.enPassantSquare, newEnPassantSquare,
                                            currentState.castleRights, newCastlingRights, *colour, *piece);

    currentState = createGameState(newPlayerTurn, newEnPassantSquare, newCastlingRights, newHalfMoves, newFullMoves, newHash);
    moveHistory.push(*move);
    gameStateHistory.push(currentState);
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

    return true;    
}