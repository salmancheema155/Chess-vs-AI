import React, { useState, useEffect } from "react"
//@ts-ignore
import createModule from "../../wasm/wasm_module.mjs"
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

/**
 * Maps piece types and colours to their corresponding piece images
 */
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

/**
 * Initial board state
 */
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

/**
 * Chess board UI with interactive pieces
 * @returns {JSX.elmeent} Chess board react component
 */
const ChessBoard = () => {
    const [board, setBoard] = useState<BoardState>(initialBoard);
    const [selectedSquare, setSelectedSquare] = useState<{row: number, col: number} | null>(null);
    const [movedFromSquare, setMovedFromSquare] = useState<{row: number, col: number} | null>(null);
    const [movedToSquare, setMovedToSquare] = useState<{row: number, col: number} | null>(null);

    const [wasm, setWasm] = useState<any>(null);
    const [legalMoveSquares, setLegalMoveSquares] = useState<Array<{row: number, col: number}>>([]);

    useEffect(() => {
        async function loadWasm() {
            const module = await createModule();
            module._initialiseGame();
            setWasm(module);
        }
        loadWasm();
    }, []);

    /**
     * Selects a square on the board
     * @param {number} rowIndex - Row the square is located in
     * @param {number} colIndex - Column the square is located in 
     */
    const handleSelectedSquare = (rowIndex: number, colIndex: number) => {
        setSelectedSquare({row: rowIndex, col: colIndex});
    };

    /**
     * Removes the currently selected square if any
     */
    const nullifySelectedSquare = () => {
        setSelectedSquare(null);
    }

    /**
     * Marks a square as the square a piece just moved from
     * @param {number} rowIndex - Row the square is located in
     * @param {number} colIndex - Column the square is located in 
     */
    const handleMovedFromSquare = (rowIndex: number, colIndex: number) => {
        setMovedFromSquare({row: rowIndex, col: colIndex});
    }

    /**
     * Marks a square as the square a piece just moved to
     * @param {number} rowIndex - Row the square is located in
     * @param {number} colIndex - Column the square is located in 
     */
    const handleMovedToSquare = (rowIndex: number, colIndex: number) => {
        setMovedToSquare({row: rowIndex, col: colIndex});
    }

    const handleLegalMoveSquares = (rowIndex: number, colIndex: number) => {
        if (!wasm) return;

        const ptr = wasm._getLegalMoves(rowIndex, colIndex);
        const jsonStr = wasm.UTF8ToString(ptr);
        const parsedMoves = JSON.parse(jsonStr).map(
            (square: {row: string, col: string}) => ({
                row: parseInt(square.row, 10),
                col: parseInt(square.col, 10)
            })
        );

        setLegalMoveSquares(parsedMoves);
    }

    const resetLegalMoveSquares = () => {
        setLegalMoveSquares([]);
    }

    /**
     * Moves a piece on the interactive board from one square to another
     * @param {{rowIndex: number, colIndex: number}} from - from Row and column of the square that the piece is currently on
     * @param {{rowIndex: number, colIndex: number}} to - to Row and column of the square that the piece is moving to
     */
    const moveSquare = (from: {rowIndex: number, colIndex: number}, to: {rowIndex: number, colIndex: number}) => {
        if (!wasm) return;

        if (wasm._makeMove(from.rowIndex, from.colIndex, to.rowIndex, to.colIndex)) {
            setBoard(prevBoard => {
                const newBoard = prevBoard.map(row => row.slice());
                newBoard[to.rowIndex][to.colIndex] = prevBoard[from.rowIndex][from.colIndex];
                newBoard[from.rowIndex][from.colIndex] = null;
                return newBoard;
            });

            handleMovedFromSquare(from.rowIndex, from.colIndex);
            handleMovedToSquare(to.rowIndex, to.colIndex);
        }

        nullifySelectedSquare();
        resetLegalMoveSquares();
    };

    const fileLabels: string[] = ["a", "b", "c", "d", "e", "f", "g", "h"];
    const rankLabels: string[] = ["8", "7", "6", "5", "4", "3", "2", "1"];

    return (
        <div className="chess-board">
            {board.map((row, rowIndex) => (
                <div className="board-row" key={rowIndex}>
                    {row.map((piece, colIndex) => {
                        const isLight = ((rowIndex + colIndex) % 2 === 0);
                        
                        const isLegalMoveSquare = legalMoveSquares.some(
                            square => square.row === rowIndex && square.col === colIndex
                        );

                        return (
                            <div 
                                key={`${rowIndex}-${colIndex}`}
                                className={`
                                    ${isLight ? "light" : "dark"}-square
                                    ${selectedSquare?.row === rowIndex && selectedSquare?.col === colIndex ? " selected-square" : ""}
                                    ${movedFromSquare?.row === rowIndex && movedFromSquare?.col === colIndex ? " moved-from-square" : ""}
                                    ${movedToSquare?.row === rowIndex && movedToSquare?.col === colIndex ? " moved-to-square" : ""}
                                    ${isLegalMoveSquare ? " legal-move-square" : ""}
                                `}
                                onClick={() => {
                                    if (piece) {
                                        if (selectedSquare) {
                                            if (piece.colour === board[selectedSquare.row][selectedSquare.col]!.colour) {
                                                if (rowIndex === selectedSquare.row && colIndex === selectedSquare.col) {
                                                    // User clicks the currently selected piece
                                                    nullifySelectedSquare();
                                                    resetLegalMoveSquares();
                                                } else {
                                                    // User clicks on a piece of the same colour as the currently selected piece
                                                    handleSelectedSquare(rowIndex, colIndex);
                                                    handleLegalMoveSquares(rowIndex, colIndex);
                                                }
                                            } else {
                                                // User makes a capture move
                                                moveSquare({rowIndex: selectedSquare.row, colIndex:selectedSquare.col}, 
                                                            {rowIndex: rowIndex, colIndex: colIndex});
                                            }
                                        } else {
                                            // User clicks on a piece without a currently selected piece
                                            handleSelectedSquare(rowIndex, colIndex);
                                            handleLegalMoveSquares(rowIndex, colIndex);
                                        }
                                    } else {
                                        if (selectedSquare) {
                                            // User makes a non capture move
                                            moveSquare({rowIndex: selectedSquare.row, colIndex:selectedSquare.col}, 
                                                        {rowIndex: rowIndex, colIndex: colIndex});
                                        }
                                    }
                                }}
                            >
                                {(rowIndex == 7 || colIndex == 0) && (
                                    <>
                                        <span className="file-corner-label">
                                            {rowIndex === 7 ? fileLabels[colIndex] : ""}
                                        </span>
                                        <span className="rank-corner-label">
                                            {colIndex === 0 ? rankLabels[rowIndex] : ""}
                                        </span>
                                    </>
                                )}

                                {piece && (
                                    <img
                                        className="square-piece"
                                        src={pieceImages[piece.colour][piece.type]}
                                        alt={`${piece.colour} ${piece.type}`}
                                    />
                                )}
                            </div>
                        )
                    })}
                </div>
            ))}
        </div>
    )
};

export default ChessBoard;