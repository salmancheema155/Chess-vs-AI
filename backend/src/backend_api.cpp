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
    const char* promotionToString(uint8_t promotion) {
        switch (promotion) {
            case Move::NO_PROMOTION: return "NO_PROMOTION";
            case Chess::toIndex(Chess::PieceType::QUEEN): return "QUEEN";
            case Chess::toIndex(Chess::PieceType::ROOK): return "ROOK";
            case Chess::toIndex(Chess::PieceType::BISHOP): return "BISHOP";
            case Chess::toIndex(Chess::PieceType::KNIGHT): return "KNIGHT";
            default: return "INVALID";
        }
    }

    const char* castlingToString(uint8_t castling) {
        switch (castling) {
            case Move::NO_CASTLE: return "NO_CASTLE";
            case Chess::toIndex(Chess::Castling::KINGSIDE): return "KINGSIDE";
            case Chess::toIndex(Chess::Castling::QUEENSIDE): return "QUEENSIDE";
            default: return "INVALID";
        }
    }

    const char* enPassantToString(uint8_t enPassant) {
        return (enPassant == Move::NO_EN_PASSANT) ? "NO_EN_PASSANT" : "EN_PASSANT";
    }

    std::string movesToJson(const std::vector<Move>& moves) {
        std::ostringstream json;
        json << "[";

        for (size_t i = 0; i < moves.size(); i++) {
            uint8_t fromSquare = moves[i].getFromSquare();
            uint8_t toSquare = moves[i].getToSquare();

            int fromRow = 7 - static_cast<int>(Board::getRank(fromSquare));
            int fromCol = static_cast<int>(Board::getFile(fromSquare));
            int toRow = 7 - static_cast<int>(Board::getRank(toSquare));
            int toCol = static_cast<int>(Board::getFile(toSquare));

            uint8_t promotion = moves[i].getPromotionPiece();
            uint8_t castling = moves[i].getCastling();
            uint8_t enPassant = moves[i].getEnPassant();

            json << "{" <<
            "\"from\":{\"row\":" << fromRow << ",\"col\":" << fromCol << "}," <<
            "\"to\":{\"row\":" << toRow << ",\"col\":" << toCol << "}," <<
            "\"promotion\":" << "\"" << promotionToString(promotion) << "\"," <<
            "\"castling\":" << "\"" << castlingToString(castling) << "\"," <<
            "\"enPassant\":" << "\"" << enPassantToString(enPassant) << "\"}";

            if (i != moves.size() - 1) json << ",";
        }
        
        json << "]";
        return json.str();
    }

    bool isValidSquare(int row, int col) {
        return (row >= 0 && row <= 7 && col >= 0 && col <= 7);
    }

    uint8_t getSquare(int row, int col) {
        return 8 * (7 - row) + col;
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
    int getMoveType(int fromRow, int fromCol, int toRow, int toCol) {
        if (!isValidSquare(fromRow, fromCol) | !isValidSquare(toRow, toCol)) return -2;

        uint8_t fromSquare = getSquare(fromRow, fromCol);
        uint8_t toSquare = getSquare(toRow, toCol);

        return game.getMoveType(fromSquare, toSquare);
    }

    EMSCRIPTEN_KEEPALIVE
    bool makeMove(int fromRow, int fromCol, int toRow, int toCol, uint8_t promotion) {
        if (!isValidSquare(fromRow, fromCol) || !isValidSquare(toRow, toCol)) return false;

        uint8_t fromSquare = getSquare(fromRow, fromCol);
        uint8_t toSquare = getSquare(toRow, toCol);

        return game.makeMove(fromSquare, toSquare, promotion);
    }

    EMSCRIPTEN_KEEPALIVE
    void undo() {
        game.undo();
    }
}

#endif // __EMSCRIPTEN__