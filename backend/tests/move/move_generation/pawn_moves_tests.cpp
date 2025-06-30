#include <gtest/gtest.h>
#include <cstdint>
#include <optional>
#include <array>
#include <vector>
#include <algorithm>
#include "board/board.h"
#include "move/move.h"
#include "move/move_generator.h"
#include "tests/move/move_debug.h"
#include "tests/board/board_debug.h"
#include "chess_types.h"

using Chess::Bitboard;
using ChessMove::Move;
using ChessMove::makeMove;
using Colour = Chess::PieceColour;
using Piece = Chess::PieceType;

namespace {
    // Sorts moves in ascending order by the .toSquare
    void sortMoves(std::vector<Move>& legalMoves) {
        std::sort(legalMoves.begin(), legalMoves.end(), [](const Move& a, const Move& b) {
            return a.toSquare < b.toSquare;
        });
    }
}

TEST(MoveGeneratorTest, checkLegalPawnMoves) {
    Board b;
    const Piece piece = Piece::PAWN;
    /*
    1. e4 e5
    2. Nf3 Nc6
    3. Bc4 Nf6
    4. Nc3 Bc5
    5. d3 O-O
    6. Bg5 h6
    7. Bxf6 Qxf6
    8. Nd5 Qd8
    9. O-O d6
    10. c3 a5
    11. d4
    */
    b.setCustomBoardState("R..Q.RK./PP...PPP/..P..N../..BPP.../p.bNp.../..np...p/.pp..pp./r.bq.rk. b ---- -- 0 11");
    // printBoard(b); // View visual board for test

    // White pawn on d4
    Colour colour = Colour::WHITE;
    uint8_t square = 27;
    std::vector<Move> legalMoves = MoveGenerator::legalMoves(b, piece, colour, square);
    std::vector<Move> expectedLegalMoves = {makeMove(piece, colour, square, square + 7, std::optional<uint8_t>(square + 7)), 
                                            makeMove(piece, colour, square, square + 9, std::optional<uint8_t>(square + 9))};
    sortMoves(legalMoves);
    EXPECT_EQ(legalMoves, expectedLegalMoves) << "legalMoves differ from expectedLegalMoves";
}