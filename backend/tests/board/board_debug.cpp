#ifndef NDEBUG // Only compile in debug builds

#include <iostream>
#include <regex>
#include <cassert>
#include <array>
#include <string>
#include <cstdint>
#include <cctype>
#include <unordered_map>
#include "board/board.h"
#include "chess_types.h"

#define UINT8_T_MAX (0xFF)

using Chess::toIndex;
using Chess::Castling;
using Chess::Bitboard;
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
            R"(^([PNBRQKpnbrqk1-9]+/){7})" // 7 repetitions of (8 piece characters followed by /)
            R"([PNBRQKpnbrqk1-9]+)" // 8 piece characters
            R"( (w|b))" // Current player turn
            R"( (-|(K?Q?k?q?)))" // Castling information
            R"( ([a-h][36]|-))" // En passant square
            R"( \d+)" // Half move number
            R"( \d+$)" // Full move number
        );

        return std::regex_match(boardState, pattern);
    }

    uint8_t algebraicToSquare(std::string algString) {
        return (algString[0] - 'a') + 8 * (algString[1] - '1');
    }

    std::string enPassantTargetToEnPassantSquare(std::string enPassantTarget) {
        char targetRank = enPassantTarget[1];
        char rank = (targetRank == '3') ? '4' : '6';
        return std::string() + enPassantTarget[0] + rank;
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

void Board::setCustomBoardState(const char* fen) {
    assert(isValidBoardState(fen) && "string fen has incorrect format");
    pieceBitboards = {{{0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL},
                        {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}}};

    whitePiecesBitboard = 0ULL;
    blackPiecesBitboard = 0ULL;
    piecesBitboard = 0ULL;

    std::unordered_map<char, Bitboard*> charMap;
    charMap['P'] = &pieceBitboards[0][0];
    charMap['N'] = &pieceBitboards[0][1];
    charMap['B'] = &pieceBitboards[0][2];
    charMap['R'] = &pieceBitboards[0][3];
    charMap['Q'] = &pieceBitboards[0][4];
    charMap['K'] = &pieceBitboards[0][5];
    charMap['p'] = &pieceBitboards[1][0];
    charMap['n'] = &pieceBitboards[1][1];
    charMap['b'] = &pieceBitboards[1][2];
    charMap['r'] = &pieceBitboards[1][3];
    charMap['q'] = &pieceBitboards[1][4];
    charMap['k'] = &pieceBitboards[1][5];

    uint8_t index = 0;
    uint8_t row = 7;
    uint8_t col = 0;
    while (fen[index] != ' ') {
        if (fen[index] == '/') {
            row--;
            col = 0;
        } else if (std::isdigit(fen[index])) {
            uint8_t digit = fen[index] - '0';
            col += digit;
        } else {
            uint8_t square = 8 * row + col;
            Bitboard* pPieceBitboard = charMap[fen[index]];
            setBit(*pPieceBitboard, square);
            std::isupper(fen[index]) ? setBit(whitePiecesBitboard, square) : setBit(blackPiecesBitboard, square);
            col++;
        }
        index++;
    }
    piecesBitboard = whitePiecesBitboard | blackPiecesBitboard;

    index += 3; // Jump to castling rights
    castlingRights[toIndex(Colour::WHITE)][toIndex(Castling::KINGSIDE)] = false;
    castlingRights[toIndex(Colour::WHITE)][toIndex(Castling::QUEENSIDE)] = false;
    castlingRights[toIndex(Colour::BLACK)][toIndex(Castling::KINGSIDE)] = false;
    castlingRights[toIndex(Colour::BLACK)][toIndex(Castling::QUEENSIDE)] = false;
    if (fen[index] != '-') {
        while (fen[index] != ' ') {
            switch (fen[index]) {
                case 'K':
                    castlingRights[toIndex(Colour::WHITE)][toIndex(Castling::KINGSIDE)] = true;
                    break;
                case 'Q':
                    castlingRights[toIndex(Colour::WHITE)][toIndex(Castling::QUEENSIDE)] = true;
                    break;
                case 'k':
                    castlingRights[toIndex(Colour::BLACK)][toIndex(Castling::KINGSIDE)] = true;
                    break;
                case 'q':
                    castlingRights[toIndex(Colour::BLACK)][toIndex(Castling::QUEENSIDE)] = true;
                    break;
            }
            index++;
        }
    } else {
        index++;
    }

    index++; // Go to en passant target square
    if (fen[index] != '-') {
        std::string enPassantTargetString(fen + index, 2);
        std::string enPassantSquareString = enPassantTargetToEnPassantSquare(enPassantTargetString);
        enPassantSquare = algebraicToSquare(enPassantSquareString);
    } else {
        enPassantSquare = std::nullopt;
    }
}

#endif // NDEBUG