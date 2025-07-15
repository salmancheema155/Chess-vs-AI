#ifdef __EMSCRIPTEN__

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include "emscripten.h"
#include "move/move.h"
#include "move/move_info.h"
#include "board/board.h"
#include "game/game.h"
#include "chess_types.h"

static Game game;
static std::string legalMovesJson;
static std::string moveInfoJson;

namespace {
    const char* pieceToString(uint8_t piece) {
        switch (piece) {
            case Chess::toIndex(Chess::PieceType::PAWN): return "PAWN";
            case Chess::toIndex(Chess::PieceType::KNIGHT): return "KNIGHT";
            case Chess::toIndex(Chess::PieceType::BISHOP): return "BISHOP";
            case Chess::toIndex(Chess::PieceType::ROOK): return "ROOK";
            case Chess::toIndex(Chess::PieceType::QUEEN): return "QUEEN";
            case Chess::toIndex(Chess::PieceType::KING): return "KING";
            default: return "NONE";
        }
    }

    const char* colourToString(uint8_t colour) {
        if (colour == Chess::toIndex(Chess::PieceColour::WHITE)) return "WHITE";
        else if (colour == Chess::toIndex(Chess::PieceColour::BLACK)) return "BLACK";
        else return "NONE";
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

    std::string legalMovesToJson(const std::vector<Move>& moves) {
        std::ostringstream json;
        json << "[";

        for (size_t i = 0; i < moves.size(); i++) {
            uint8_t toSquare = moves[i].getToSquare();

            int toRow = 7 - static_cast<int>(Board::getRank(toSquare));
            int toCol = static_cast<int>(Board::getFile(toSquare));

            json << "{\"row\":" << toRow << ",\"col\":" << toCol << "}";

            if (i != moves.size() - 1) json << ",";
        }
        
        json << "]";
        return json.str();
    }

    std::string moveInfoToJson(const MoveInfo& moveInfo) {
        const char* pieceStr = pieceToString(moveInfo.movedPiece);
        const char* colourStr = colourToString(moveInfo.movedPieceColour);
        const char* capturedPieceStr = pieceToString(moveInfo.capturedPiece);
        const char* capturedColourStr = pieceToString(moveInfo.capturedPieceColour);
        
        uint8_t fromSquare = moveInfo.move.getFromSquare();
        uint8_t toSquare = moveInfo.move.getToSquare();

        int fromRow = 7 - static_cast<int>(Board::getRank(fromSquare));
        int fromCol = static_cast<int>(Board::getFile(fromSquare));
        int toRow = 7 - static_cast<int>(Board::getRank(toSquare));
        int toCol = static_cast<int>(Board::getFile(toSquare));

        const char* castlingStr = castlingToString(moveInfo.move.getCastling());
        bool promotion = (moveInfo.move.getPromotionPiece() != Move::NO_PROMOTION);
        bool enPassant = (moveInfo.move.getEnPassant() != Move::NO_EN_PASSANT);

        std::ostringstream json;

        json << "{" <<
        "\"from\":{\"row\":" << fromRow << ",\"col\":" << fromCol << "}," <<
        "\"to\":{\"row\":" << toRow << ",\"col\":" << toCol << "}," <<
        "\"piece\":\"" << pieceStr << "\"," <<
        "\"colour\":\"" << colourStr << "\"," <<
        "\"capturedPiece\":\"" << capturedPieceStr << "\"," <<
        "\"capturedColour\":\"" << capturedColourStr << "\"," <<
        "\"castling\":\"" << castlingStr << "\"," <<
        "\"promotion\":" << (promotion ? "true": "false") << "," <<
        "\"enPassant\":" << (enPassant ? "true": "false") <<
        "}";

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
    int getCurrentTurn() {
        return Chess::toIndex(game.getCurrentTurn());
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
        legalMovesJson = legalMovesToJson(game.getLegalMoves(square));
        return legalMovesJson.c_str();
        
    }

    EMSCRIPTEN_KEEPALIVE
    const char* getMoveInfo(int fromRow, int fromCol, int toRow, int toCol) {
        if (!isValidSquare(fromRow, fromCol) | !isValidSquare(toRow, toCol)) return "{\"error\": \"Invalid square input\"}";

        uint8_t fromSquare = getSquare(fromRow, fromCol);
        uint8_t toSquare = getSquare(toRow, toCol);

        std::optional<MoveInfo> moveInfoOpt = game.getMoveInfo(fromSquare, toSquare);
        if (!moveInfoOpt.has_value()) return "{\"error\": \"No legal move exists\"}";

        moveInfoJson = moveInfoToJson(*moveInfoOpt);
        return moveInfoJson.c_str();
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