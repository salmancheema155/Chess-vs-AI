export interface Move {
    from: {row: number, col: number};
    to: {row: number, col: number};
    promotionPiece: string;
    castling: string;
    enPassant: string;
};