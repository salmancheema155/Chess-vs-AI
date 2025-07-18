export interface Move {
    from: {row: number, col: number};
    to: {row: number, col: number};
    piece: string;
    colour: string;
    capturedPiece: string;
    capturedColour: string;
    castling: string;
    promotion: string;
    enPassant: boolean;
};