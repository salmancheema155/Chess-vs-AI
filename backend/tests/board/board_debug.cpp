#ifndef NDEBUG // Only compile in debug builds

#include <iostream>
#include <regex>
#include <cassert>
#include <array>
#include <string>
#include "board/board.h"
#include "chess_types.h"

#define UINT8_T_MAX (0xFF)

using Chess::toIndex;
using Piece = Chess::PieceType;
using Colour = Chess::PieceColour;

namespace {
    constexpr const char* white_pawn = "P";
    constexpr const char* white_knight = "N";
    constexpr const char* white_bishop = "B";
    constexpr const char* white_rook = "R";
    constexpr const char* white_queen = "Q";
    constexpr const char* white_king = "K";

    constexpr const char* black_pawn = "p";
    constexpr const char* black_knight = "n";
    constexpr const char* black_bishop = "b";
    constexpr const char* black_rook = "r";
    constexpr const char* black_queen = "q";
    constexpr const char* black_king = "k";

    constexpr const char* pieces[2][6] = {{white_pawn, white_knight, white_bishop, white_rook, white_queen, white_king}, 
                                        {black_pawn, black_knight, black_bishop, black_rook, black_queen, black_king}};

    void printSquare(const Board& board, uint8_t square) {
        auto [piece, colour] = board.getPieceAndColour(square);
        if (piece) { // piece exists at this square
            std::cout << pieces[toIndex(*colour)][toIndex(*piece)];
        } else {
            std::cout << " ";
        }
    }
}

namespace {
    void setBit(uint64_t& num, uint8_t shift) {
        num |= (1ULL << shift);
    }

    bool isValidBoardState(const char* boardState) {
        static const std::regex pattern(
            R"(^([PNBRQKpnbrqk\.]{8}/){7})" // 7 repetitions of (8 piece characters followed by /)
            R"([PNBRQKpnbrqk\.]{8})" // 8 piece characters
            R"( (w|b))" // Current player turn
            R"( (K|-)(Q|-)(k|-)(q|-))" // Castling information
            R"( ([a-h][36]|--))" // En passant square
            R"( \d+)" // Half move number
            R"( \d+$)" // Full move number
        );

        return std::regex_match(boardState, pattern);
    }

    uint8_t algebraicToSquare(std::string algString) {
        if (algString == "--") return UINT8_T_MAX;
        return (algString[0] - 'a') + 8 * (algString[1] - '1');
    }
}

void printBoard(const Board& board, char delimiter) {
    for (int row = 7; row >= 0; row--) {
        for (int col = 0; col < 8; col++) {
            printSquare(board, 8 * row + col);
            std::cout << delimiter;
        }
        std::cout << "\n";
    }
}

/*
Parses a custom board state string similar to FEN notation

Format: 
- 8 rows (ranks) separated by '/' from rank 1 (bottom) to rank 8 (top)
- Each row must contain exactly 8 characters
- Lower case characters represent black pieces, upper case represent white pieces

- Piece Characters:
    - p, n, b, r, q, k represent black pawn, knight, bishop, rook, queen or king respectively
    - P, N, B, R, Q, K represent white pawn, knight, bishop, rook, queen or king respectively
    - '.' character denotes an empty square

- After the board representation, the following fields must be included separated by a single space
    - Current player turn: 'w' or 'b' character denoting the current player's turn
    - Castling rights: 'K', 'Q', 'k', 'q' or '-' characters to denote black and white's kingside and queenside castle rights
       where '-' denotes illegibility
    - En passant square: Current square of the piece that just moved 2 squares forward represented in algebraic notation (e.g. e4)
       where '--' denotes no such piece
    - Half move number: Number of half moves elapsed since the last capture or pawn advance
    - Full move number: Number of full moves elapsed since the start of the game starting at 1 and incremented after black's turn

Examples:
- "RNBQKBNR/PPPPPPPP/......../......../......../......../pppppppp/rnbqkbnr w KQkq -- 0 1"
- "R..Q.RK./PP...PPP/..P..N../..BPP.../p.bNp.../..np...p/.pp..pp./r.bq.rk. b ---- -- 0 11"

This function should only be used in testing and debugging
*/
void Board::setCustomBoardState(const char* boardState) {
    assert(isValidBoardState(boardState) && "string boardState has incorrect format");
    pieceBitboards = {{{0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL},
                        {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}}};

    whitePiecesBitboard = 0ULL;
    blackPiecesBitboard = 0ULL;
    piecesBitboard = 0ULL;
    
    constexpr char charMap[2][6] = {{'P', 'N', 'B', 'R', 'Q', 'K'},
                                    {'p', 'n', 'b', 'r', 'q', 'k'}};
    const int boardChars = 71;
    const char currTurnChar = boardState[72];
    const char whiteKingsideChar = boardState[74];
    const char whiteQueensideChar = boardState[75];
    const char blackKingsideChar = boardState[76];
    const char blackQueensideChar = boardState[77];
    const std::string enPassantSquareString(boardState + 79, 2);

    for (uint8_t i = 0; i < 2; i++) {
        for (uint8_t j = 0; j < toIndex(Piece::COUNT); j++) {
            char symbol = charMap[i][j];
            for (int k = 0; k < boardChars; k++) {
                if (boardState[k] == symbol) {
                    uint8_t row = k / 9, col = k % 9;
                    uint8_t square = 8 * row + col;
                    setBit(pieceBitboards[i][j], square);
                    if (i == 0) {
                        setBit(whitePiecesBitboard, square);
                    } else {
                        setBit(blackPiecesBitboard, square);
                    }
                }
            }
        }
    }
    piecesBitboard = whitePiecesBitboard | blackPiecesBitboard;

    currTurn = (currTurnChar == 'w') ? Colour::WHITE : Colour::BLACK;

    kingsideCastle[0] = (whiteKingsideChar == 'K') ? true : false;
    kingsideCastle[1] = (blackKingsideChar == 'k') ? true : false;
    queensideCastle[0] = (whiteQueensideChar == 'Q') ? true : false;
    queensideCastle[1] = (blackQueensideChar == 'q') ? true : false;

    uint8_t square = algebraicToSquare(enPassantSquareString);
    enPassantSquare = (square == UINT8_T_MAX) ? std::nullopt : std::optional<uint8_t>(square);
}

#endif // NDEBUG