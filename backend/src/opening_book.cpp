#include <cstdint>
#include <cstddef>
#include <cctype>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include "book/opening_book_data.h"
#include "book/opening_book.h"
#include "move/move.h"
#include "board/board.h"
#include "zobrist_hash.h"
#include "chess_types.h"

using Piece = Chess::PieceType;
using Chess::Castling;
using Chess::toIndex;

namespace {
    Move uciToMove(std::string uciMove) {
        uint8_t from = (uciMove[0] - 'a') + 8 * (uciMove[1] - '1');
        uint8_t to = (uciMove[2] - 'a') + 8 * (uciMove[3] - '1');
        uint8_t promotion = Move::NO_PROMOTION;

        // Is promotion move
        if (uciMove.size() == 5) {
            char promotionChar = std::tolower(uciMove[4]);
            switch (promotionChar) {
                case 'q':
                    promotion = toIndex(Piece::QUEEN);
                    break;
                case 'r':
                    promotion = toIndex(Piece::ROOK);
                    break;
                case 'b':
                    promotion = toIndex(Piece::BISHOP);
                    break;
                case 'n':
                    promotion = toIndex(Piece::KNIGHT);
                    break;
            }
        }

        Move move(from, to, Move::NO_CAPTURE, promotion);
        
        return move;
    }
}

void OpeningBook::loadBook() {
    book.reserve(10000);

    for (int i = 0; i < OpeningBookData::bookSize; i++) {
        const auto& entry = OpeningBookData::book[i];
        uint64_t hash = Zobrist::computeHash(entry.fen);

        std::stringstream ss(entry.moves);
        std::string uciMove;
        std::vector<Move> moves;

        while (std::getline(ss, uciMove, ',')) {
            moves.push_back(uciToMove(uciMove));
        }

        book[hash] = moves;
    }
}

Move OpeningBook::getMove(uint64_t hash) {
    auto iterator = book.find(hash);
    if (iterator == book.end() || iterator->second.empty()) {
        return Move();
    }

    const std::vector<Move>& moves = iterator->second;

    std::uniform_int_distribution<std::size_t> distribution(0, moves.size() - 1);
    return moves[distribution(rng)];
}

Move OpeningBook::getMove(uint64_t hash, Board& board) {
    Move move = getMove(hash);
    if (move == Move()) return Move();

    uint8_t from = move.getFromSquare();
    uint8_t to = move.getToSquare();
    Piece piece = board.getPiece(from);
    Piece target = board.getPiece(to);

    if (piece == Piece::KING) {
        // Kingside castling
        if ((from == 4 && to == 6) || (from == 60 && to == 62)) {
            move.setCastling(toIndex(Castling::KINGSIDE));
        // Queenside castling
        } else if ((from == 4 && to == 2) || (from == 60 && to == 58)) {
            move.setCastling(toIndex(Castling::QUEENSIDE));
        }
    }

    if (piece == Piece::PAWN) {
        // Pawn moving diagonally to an empty square = en passant
        if (Board::getFile(from) != Board::getFile(to) && target == Piece::NONE) {
            move.setEnPassant(1);
            move.setCapturedPiece(toIndex(Piece::PAWN));
        }
    }

    if (target != Piece::NONE) {
        move.setCapturedPiece(toIndex(target));
    }

    return move;
}