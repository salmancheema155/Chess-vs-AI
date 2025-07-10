export type Colour = "white" | "black";

export type PieceType = "pawn" | "knight" | "bishop" | "rook" | "queen" | "king";

export interface Piece {
    type: PieceType;
    colour: Colour;
}