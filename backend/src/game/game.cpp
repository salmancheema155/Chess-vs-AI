#include <stack>
#include "game/game.h"
#include "game/game_state.h"
#include "board/board.h"
#include "zobrist_hash.h"
#include "chess_types.h"

using Colour = Chess::PieceColour;

Game::Game() {
    Colour currentTurn = Colour::WHITE;
    uint64_t hash = Zobrist::computeInitialHash(board, currentTurn);
    gameStateHistory.push(createGameState(currentTurn, board.getEnPassantSquare(), board.getCastlingRights(), 0, 1, hash));
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