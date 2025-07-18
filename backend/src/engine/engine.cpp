#include <vector>
#include <cstdint>
#include "engine/engine.h"
#include "board/board.h"
#include "move/move.h"
#include "game/game.h"
#include "move/move_generator.h"
#include "chess_types.h"

#include <cstdlib>
#include <ctime>

using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;

Move Engine::getMove(Game& game, int depth) {
    Board board = game.getBoard();
    std::vector<Move> moves;
    std::vector<uint8_t> occupiedSquares = board.getSquares(Colour::BLACK);

    for (uint8_t square : occupiedSquares) {
        Piece piece = board.getPiece(square);
        std::vector<Move> pieceMoves = MoveGenerator::legalMoves(board, piece, Colour::BLACK, square);
        moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
    }

    std::srand(std::time(nullptr));
    int index = std::rand() % moves.size();

    return moves[index];
}

// int Engine::minimax(Game& game, int depth, int alpha, int beta, Chess::PieceColour colour) {
//     if (depth == 0) return evaluate(game, game.board);
//     GameStateEvaluation state = game.getCurrentGameStateEvaluation();
//     if (state != GameStateEvaluation::IN_PROGRESS && state != GameStateEvaluation::CHECK) {
//         return evaluate(game);
//     }

    
// }

// int Engine::evaluate(Game& game) {

// }