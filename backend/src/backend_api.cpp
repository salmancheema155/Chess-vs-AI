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
            uint8_t fromSquare = moves[i].getFromSquare();
            uint8_t toSquare = moves[i].getToSquare();

            uint8_t fromRow = Board::getRank(fromSquare);
            uint8_t fromCol = Board::getFile(fromSquare);
            uint8_t toRow = Board::getRank(toSquare);
            uint8_t toCol = Board::getFile(toSquare);

            json << "{\"from\":";
            json << "{\"row\":";
            json << std::to_string(static_cast<int>(fromRow));
            json << ",\"col\":";
            json << std::to_string(static_cast<int>(fromCol));
            json << "},";
            json << "\"to\":";
            json << "{\"row\":";
            json << std::to_string(static_cast<int>(toRow));
            json << ",\"col\":";
            json << std::to_string(static_cast<int>(toCol));
            json << "}}";

            if (i != moves.size() - 1) json << ",";
        }
        
        json << "]";
        return json.str();
    }

    bool isValidSquare(const char* square) {
        if (!square || strlen(square) != 2) return false;

        char file = square[0];
        char rank = square[1];

        return file >= 'a' && file <= 'h' && 
               rank >= '1' && rank <= '8';
    }

    uint8_t algebraicToSquare(const char* square) {
        char file = square[0];
        char rank = square[1];

        return (file - 'a') + 8 * (rank - '1');
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void initialiseGame() {
        game = Game();
    }

    EMSCRIPTEN_KEEPALIVE
    bool isCurrentPlayerOccupies(const char* square) {
        if (!isValidSquare(square)) return false;

        uint8_t currSquare = algebraicToSquare(square);
        return game.isCurrentPlayerOccupies(currSquare);
    }

    EMSCRIPTEN_KEEPALIVE
    const char* getLegalMoves(const char* square) {
        if (!isValidSquare(square)) return "{\"error\": \"Invalid square input\"}";

        uint8_t currSquare = algebraicToSquare(square);
        legalMovesJson = movesToJson(game.getLegalMoves(currSquare));
        return legalMovesJson.c_str();
        
    }

    EMSCRIPTEN_KEEPALIVE
    bool makeMove(const char* fromSquare, const char* toSquare) {
        if (!isValidSquare(fromSquare) || !isValidSquare(toSquare)) return false;

        uint8_t from = algebraicToSquare(fromSquare);
        uint8_t to = algebraicToSquare(toSquare);

        return game.makeMove(from, to);
    }

    EMSCRIPTEN_KEEPALIVE
    void undo() {
        game.undo();
    }
}

#endif // __EMSCRIPTEN__