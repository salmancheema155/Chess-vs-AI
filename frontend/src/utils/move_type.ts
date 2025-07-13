export const numberToMoveType = (flag: number) => {
    switch (flag) {
        case -2: return "INVALID ROW/COLUMN TO BACKEND API";
        case -1: return "ILLEGAL";
        case 0: return "NORMAL";
        case 1: return "KINGSIDE_CASTLE";
        case 2: return "QUEENSIDE_CASTLE";
        case 3: return "PROMOTION";
        case 4: return "EN_PASSANT";
    }
}