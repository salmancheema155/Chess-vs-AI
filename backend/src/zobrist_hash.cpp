#include <cstdint>
#include <utility>
#include "zobrist_hash.h"
#include "zobrist_keys.h"
#include "board/board.h"
#include "chess_types.h"

using Chess::toIndex;

namespace Zobrist {
    
    uint64_t computeInitialHash(Board& board) {
        uint64_t hash = 0;
        for (uint8_t square = 0; square < 64; square++) {
            auto [piece, colour] = board.getPieceAndColour(square);
            
            if (piece.has_value() && colour.has_value()) {
                uint64_t key = zobristTable[toIndex(*colour)][toIndex(*piece)][square];
                hash ^= key;
            }
        }

        
    }

    uint64_t updateHash(uint64_t currentHash, const Move& move, const GameState& oldGameState, const GameState& newGameState) {
        
    }
}