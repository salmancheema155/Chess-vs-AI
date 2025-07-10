import React, { useState } from "react"
import whitePawn from "../../assets/pieces/white_pawn.svg"
import whiteKnight from "../../assets/pieces/white_knight.svg"
import whiteBishop from "../../assets/pieces/white_bishop.svg"
import whiteRook from "../../assets/pieces/white_rook.svg"
import whiteQueen from "../../assets/pieces/white_queen.svg"
import whiteKing from "../../assets/pieces/white_king.svg"
import blackPawn from "../../assets/pieces/black_pawn.svg"
import blackKnight from "../../assets/pieces/black_knight.svg"
import blackBishop from "../../assets/pieces/black_bishop.svg"
import blackRook from "../../assets/pieces/black_rook.svg"
import blackQueen from "../../assets/pieces/black_queen.svg"
import blackKing from "../../assets/pieces/black_king.svg"
import type { Piece } from "../../types/piece.ts"
import "./chessboard.css"

type BoardState = (Piece | null)[][];

const pieceImages = {
    white: {
        pawn: whitePawn,
        knight: whiteKnight,
        bishop: whiteBishop,
        rook: whiteRook,
        queen: whiteQueen,
        king: whiteKing
    },
    black: {
        pawn: blackPawn,
        knight: blackKnight,
        bishop: blackBishop,
        rook: blackRook,
        queen: blackQueen,
        king: blackKing
    }
};

const initialBoard: BoardState = [
    [
        {type: "rook", colour: "black"},
        {type: "knight", colour: "black"},
        {type: "bishop", colour: "black"},
        {type: "queen", colour: "black"},
        {type: "king", colour: "black"},
        {type: "bishop", colour: "black"},
        {type: "knight", colour: "black"},
        {type: "rook", colour: "black"}
    ],
    Array(8).fill(null).map(() => ({type: "pawn", colour: "black"})),
    ...Array(4).fill(null).map(() => (Array(8).fill(null))),
    Array(8).fill(null).map(() => ({type: "pawn", colour: "white"})),
    [
        {type: "rook", colour: "white"},
        {type: "knight", colour: "white"},
        {type: "bishop", colour: "white"},
        {type: "queen", colour: "white"},
        {type: "king", colour: "white"},
        {type: "bishop", colour: "white"},
        {type: "knight", colour: "white"},
        {type: "rook", colour: "white"},
    ]
];

const ChessBoard = () => {
    const [board, setBoard] = useState<BoardState>(initialBoard);

    return (
        <div className="chess-board">
            {board.map((row, rowIndex) => (
                <div className="board-row" key={rowIndex}>
                    {row.map((piece, colIndex) => {
                        const isLight = ((rowIndex + colIndex) % 2 === 0);
                        return (
                            <div className={`${isLight ? "light" : "dark"}-square`} key={`${rowIndex}-${colIndex}`}>
                                {piece && (
                                    <img
                                        src={pieceImages[piece.colour][piece.type]}
                                        alt="k"
                                        style={{width: "100%", height: "100%"}}
                                    />
                                )}
                            </div>
                        )
                    }
                    )}
                </div>
            ))}
        </div>
    )
};

export default ChessBoard;