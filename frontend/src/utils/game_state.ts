export const numberToGameState = (gameStateNum: number) => {
    switch (gameStateNum) {
        case 0: return "IN_PROGRESS";
        case 1: return "CHECKMATE";
        case 2: return "STALEMATE";
        case 3: return "CHECK";
        case 4: return "DRAW_BY_REPETITION";
        case 5: return "DRAW_BY_INSUFFICIENT_MATERIAL";
        case 6: return "DRAW_BY_FIFTY_MOVE_RULE";
        default: return "INVALID";
    }
}