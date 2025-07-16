import React, { useState, useEffect, useRef } from "react"
//@ts-ignore
import createModule from "../../wasm/wasm_module.mjs"
import PromotionModal from "../promotion_modal/promotion_modal.tsx"
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
import type { Piece, Colour } from "../../types/piece.ts"
import type { Move } from "../../types/move.ts"
import { promotionPieceToNumber } from "../../utils/promotion.ts"
import { numberToColour } from "../../utils/colour.ts"
import { numberToGameState } from "../../utils/game_state.ts"
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
    const [currentTurn, setCurrentTurn] = useState<Colour>("white");
    const [gameState, setGameState] = useState<string>("IN_PROGRESS");

    const [wasm, setWasm] = useState<any>(null);
    const [legalMoveSquares, setLegalMoveSquares] = useState<Array<{row: number, col: number}>>([]);
    const [promotionInfo, setPromotionInfo] = useState<{
        from: {rowIndex: number, colIndex: number};
        to: {rowIndex: number, colIndex: number};
        colour: Colour;
        position: {top: number, left: number};
    } | null>(null);

    useEffect(() => {
        async function loadWasm() {
            const module = await createModule();
            module._initialiseGame();
            setWasm(module);
        }
        loadWasm();
    }, []);

    const getGameEvaluationMessage = () => {
        switch (gameState) {
            case "STALEMATE": return "Draw by stalemate";
            case "CHECK": return "Check";
            case "DRAW_BY_REPETITION": return "Draw by repetition";
            case "DRAW_BY_INSUFFICIENT_MATERIAL": return "Draw by insufficient material";
            case "DRAW_BY_FIFTY_MOVE_RULE": return "Draw by fifty move rule";
            case "CHECKMATE":
                const winner = (currentTurn == "white") ? "Black" : "White";
                return winner + " wins by checkmate!";
        }
    }

    const handleGameState = () => {
        if (!wasm) return;
        const gameStateNum : number = wasm._getCurrentGameStateEvaluation();
        const gameState = numberToGameState(gameStateNum);
        setGameState(gameState);
    }

    const handleCurrentTurn = () => {
        const currentTurnNum: number = wasm._getCurrentTurn();
        const currentTurnString = numberToColour(currentTurnNum);
        setCurrentTurn(currentTurnString as Colour);
    }

    /**
     * Selects a square on the board
     * @param {number} rowIndex - Row the square is located in
     * @param {number} colIndex - Column the square is located in 
     */
    const handleSelectedSquare = (rowIndex: number, colIndex: number) => {
        if (!wasm) return;

        if (currentTurn == board[rowIndex][colIndex]?.colour) {
            setSelectedSquare({row: rowIndex, col: colIndex});
        }
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

    /**
     * Promotes a pawn to the given piece
     * @param {string} piece - String representation of piece to promote to
     */
    const handleSelectPromotion = (piece: string) => {
        if (!promotionInfo || !wasm) return;

        const {from, to, colour, ..._} = promotionInfo;
        wasm._makeMove(from.rowIndex, from.colIndex, to.rowIndex, to.colIndex, promotionPieceToNumber(piece));

        setBoard(prevBoard => {
            const newBoard = prevBoard.map(row => row.slice());
            newBoard[to.rowIndex][to.colIndex] = {type: piece as Piece["type"], colour};
            newBoard[from.rowIndex][from.colIndex] = null;
            return newBoard;
        });

        handleMovedFromSquare(from.rowIndex, from.colIndex);
        handleMovedToSquare(to.rowIndex, to.colIndex);

        nullifySelectedSquare();
        resetLegalMoveSquares();
        setPromotionInfo(null);

        handleCurrentTurn();
        handleGameState();
    }

    /**
     * 
     * @param {number} rowIndex - Row of piece to find legal moves for
     * @param {number} colIndex - Column of piece to find legal moves for
     */
    const handleLegalMoveSquares = (rowIndex: number, colIndex: number) => {
        if (!wasm) return;

        const ptr = wasm._getLegalMoves(rowIndex, colIndex);
        const jsonStr = wasm.UTF8ToString(ptr);
        const moves : [{row: number, col: number}] = JSON.parse(jsonStr);

        if (currentTurn == board[rowIndex][colIndex]?.colour) {
            setLegalMoveSquares(moves);
        }
    }

    /**
     * Removes the highlighted legal squares from the board
     */
    const resetLegalMoveSquares = () => {
        setLegalMoveSquares([]);
    }

    /**
     * Moves a piece from one square to another
     * @param {{rowIndex: number, colIndex: number}} from - Row and column of the piece before moving 
     * @param {{rowIndex: number, colIndex: number}} to - Row and column of the piece after moving
     */
    const movePiece = (from: {rowIndex: number, colIndex: number}, 
                        to: {rowIndex: number, colIndex: number}) => {
            
        setBoard(prevBoard => {
            const newBoard = prevBoard.map(row => row.slice());
            newBoard[to.rowIndex][to.colIndex] = prevBoard[from.rowIndex][from.colIndex];
            newBoard[from.rowIndex][from.colIndex] = null;
            return newBoard;
        });
    }

    /**
     * Removes a piece from the board
     * @param rowIndex - Row that the piece is located on
     * @param colIndex - Column that the piece is located on
     */
    const removePiece = (rowIndex: number, colIndex: number) => {
        setBoard(prevBoard => {
            const newBoard = prevBoard.map(row => row.slice());
            newBoard[rowIndex][colIndex] = null;
            return newBoard;
        });
    }

    /**
     * Moves a piece on the interactive board from one square to another
     * @param {{rowIndex: number, colIndex: number}} from - from Row and column of the square that the piece is currently on
     * @param {{rowIndex: number, colIndex: number}} to - to Row and column of the square that the piece is moving to
     * @param {string} promotionPiece - The piece that is gained from promotion if any with "NONE" specifying no promotion
     */
    const moveSquare = (from: {rowIndex: number, colIndex: number}, to: {rowIndex: number, colIndex: number}) => {

        if (!wasm) return;

        const ptr = wasm._getMoveInfo(from.rowIndex, from.colIndex, to.rowIndex, to.colIndex);
        const jsonStr = wasm.UTF8ToString(ptr);
        const move: Move = JSON.parse(jsonStr);

        if (move.promotion && boardRef.current) {
            const squareSize = boardRef.current.offsetWidth / 8;
            const boardRect = boardRef.current.getBoundingClientRect();

            let top = boardRect.top + to.rowIndex * squareSize;
            if (move.colour == "BLACK") top = top + squareSize - 340;
            const left = boardRect.left + to.colIndex * squareSize;

            setPromotionInfo({from, to, colour: move.colour.toLowerCase() as Colour, position: {top, left}});
            return;
        }

        if (wasm._makeMove(from.rowIndex, from.colIndex, to.rowIndex, to.colIndex, promotionPieceToNumber("NONE"))) {
            movePiece({rowIndex: from.rowIndex, colIndex: from.colIndex}, {rowIndex: to.rowIndex, colIndex: to.colIndex});

            handleMovedFromSquare(from.rowIndex, from.colIndex);
            handleMovedToSquare(to.rowIndex, to.colIndex);

            if (move.castling == "KINGSIDE") {
                if (move.colour == "WHITE") {
                    movePiece({rowIndex: 7, colIndex: 7}, {rowIndex: 7, colIndex: 5});
                } else if (move.colour == "BLACK") {
                    movePiece({rowIndex: 0, colIndex: 7}, {rowIndex: 0, colIndex: 5});
                }
            } else if (move.castling == "QUEENSIDE") {
                if (move.colour == "WHITE") {
                    movePiece({rowIndex: 7, colIndex: 0}, {rowIndex: 7, colIndex: 3});
                } else if (move.colour == "BLACK") {
                    movePiece({rowIndex: 0, colIndex: 0}, {rowIndex: 0, colIndex: 3});
                }
            }

            if (move.enPassant) {
                removePiece(move.from.row, move.to.col);
            }

            handleCurrentTurn();
            handleGameState();
        }

        nullifySelectedSquare();
        resetLegalMoveSquares();
    };

    const fileLabels: string[] = ["a", "b", "c", "d", "e", "f", "g", "h"];
    const rankLabels: string[] = ["8", "7", "6", "5", "4", "3", "2", "1"];

    const boardRef = useRef<HTMLDivElement | null>(null);

    return (
        <>
            <div className="chess-board" ref={boardRef}>
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
                                        ${piece && piece.colour === currentTurn ? " clickable-square" : ""}
                                    `}
                                    onDragOver={(e) => e.preventDefault()}
                                    onDrop={(e) => {
                                        const from = JSON.parse(e.dataTransfer.getData("text/plain"));
                                        moveSquare({rowIndex: from.rowIndex, colIndex: from.colIndex}, {rowIndex, colIndex});
                                    }}
                                    onClick={() => {
                                        if (gameState !== "IN_PROGRESS" && gameState !== "CHECK") return;
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
                                            draggable={true}
                                            onDragStart={(e) => {
                                                if (currentTurn == board[rowIndex][colIndex]?.colour) {
                                                    e.dataTransfer.setData("text/plain", JSON.stringify({rowIndex, colIndex}));
                                                    handleSelectedSquare(rowIndex, colIndex);
                                                    handleLegalMoveSquares(rowIndex, colIndex);
                                                }
                                            }}
                                        />
                                    )}
                                </div>
                            );
                        })}
                    </div>
                ))}
            </div>

            {promotionInfo && (
                <div
                    className="promotion-backdrop"
                    onClick={() => {
                        setPromotionInfo(null);
                        nullifySelectedSquare();
                        resetLegalMoveSquares();
                    }}
                >
                    <PromotionModal
                        colour={promotionInfo.colour}
                        position={promotionInfo.position}
                        onSelect={handleSelectPromotion}
                    />
                </div>
            )}

            {gameState !== "IN_PROGRESS" && (
                <div className="game-state-banner">
                    {getGameEvaluationMessage()}
                </div>
            )}
        </>
    )
};

export default ChessBoard;