#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <cstdint>
#include <optional>
#include "board/board.h"
#include "move/move.h"
#include "chess_types.h"

struct GameState {
    Board board;
    Move move;
    Chess::PieceColour playerTurn;
    
    uint16_t halfMoveClock;
    uint16_t fullMoves;

    uint64_t hash;
};

#endif // GAME_STATE_H