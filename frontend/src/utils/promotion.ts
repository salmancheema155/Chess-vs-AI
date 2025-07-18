const KNIGHT: number = 1;
const BISHOP: number = 2;
const ROOK: number = 3;
const QUEEN: number = 4;
const NO_PROMOTION: number = 6;

export const promotionPieceToNumber = (promotionPiece: string) => {
    switch (promotionPiece.toUpperCase()) {
        case "QUEEN": return QUEEN;
        case "ROOK": return ROOK;
        case "BISHOP": return BISHOP;
        case "KNIGHT": return KNIGHT;
        default: return NO_PROMOTION;
    }
}

export const numberToPromotionPiece = (promotionNum: number) => {
    switch (promotionNum) {
        case KNIGHT: return "knight";
        case BISHOP: return "bishop";
        case ROOK: return "rook";
        case QUEEN: return "queen";
        default: return "NONE";
    }
}