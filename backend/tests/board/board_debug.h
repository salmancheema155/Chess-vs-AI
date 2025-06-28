#ifndef BOARD_DEBUG_H
#define BOARD_DEBUG_H

#ifndef NDEBUG // Only include in debug builds
#include "board/board.h"

// Prints board using unicode chess characters
// using a delimieter character to separate columns
void printBoard(const Board& board, char delimiter = '|');

#endif // NDEBUG
#endif // BOARD_DEBUG_H