#ifdef __EMSCRIPTEN__

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include "emscripten.h"
#include "move/move.h"
#include "board/board.h"
#include "game/game.h"
#include "chess_types.h"

static Game game;
static std::string legalMovesJson;

namespace {
    std::string movesToJson(const std::vector<Move>& moves) {
        std::ostringstream json;
        json << "[";

        for (size_t i = 0; i < moves.size(); i++) {
            uint8_t toSquare = moves[i].getToSquare();

            uint8_t toRow = Board::getRank(toSquare);
            uint8_t toCol = Board::getFile(toSquare);

            json << "{\"row\":";
            json << std::to_string(static_cast<int>(toRow));
            json << ",\"col\":";
            json << std::to_string(static_cast<int>(toCol));
            json << "}";

            if (i != moves.size() - 1) json << ",";
        }
        
        json << "]";
        return json.str();
    }

    bool isValidSquare(int row, int col) {
        return (row >= 0 && row <= 7 && col >= 0 && col <= 7);
    }

    uint8_t getSquare(int row, int col) {
        return 8 * row + col;
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void initialiseGame() {
        game = Game();
    }

    EMSCRIPTEN_KEEPALIVE
    bool isCurrentPlayerOccupies(int row, int col) {
        if (!isValidSquare(row, col)) return false;

        uint8_t square = getSquare(row, col);
        return game.isCurrentPlayerOccupies(square);
    }

    EMSCRIPTEN_KEEPALIVE
    const char* getLegalMoves(int row, int col) {
        if (!isValidSquare(row, col)) return "{\"error\": \"Invalid square input\"}";

        uint8_t square = getSquare(row, col);
        legalMovesJson = movesToJson(game.getLegalMoves(square));
        return legalMovesJson.c_str();
        
    }

    EMSCRIPTEN_KEEPALIVE
    bool makeMove(int fromRow, int fromCol, int toRow, int toCol) {
        if (!isValidSquare(fromRow, fromCol) || !isValidSquare(toRow, toCol)) return false;

        uint8_t fromSquare = getSquare(fromRow, fromCol);
        uint8_t toSquare = getSquare(toRow, toCol);

        return game.makeMove(fromSquare, toSquare);
    }

    EMSCRIPTEN_KEEPALIVE
    void undo() {
        game.undo();
    }
}

#endif // __EMSCRIPTEN__