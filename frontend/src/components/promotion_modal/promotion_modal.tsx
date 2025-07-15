import React from "react"
import whiteKnight from "../../assets/pieces/white_knight.svg"
import whiteBishop from "../../assets/pieces/white_bishop.svg"
import whiteRook from "../../assets/pieces/white_rook.svg"
import whiteQueen from "../../assets/pieces/white_queen.svg"
import blackKnight from "../../assets/pieces/black_knight.svg"
import blackBishop from "../../assets/pieces/black_bishop.svg"
import blackRook from "../../assets/pieces/black_rook.svg"
import blackQueen from "../../assets/pieces/black_queen.svg"
import type { Colour } from "../../types/piece.ts"
import "./promotion_modal.css"

type PromotionPiece = "queen" | "rook" | "bishop" | "knight";

interface promotionModalProps {
    colour: Colour;
    onSelect: (piece: string) => void;
    position: {top: number, left: number};
}

const PromotionModal = ({colour, onSelect, position}: promotionModalProps) => {
    const pieces: PromotionPiece[] = ["queen", "rook", "bishop", "knight"];

    const pieceImages = {
        white: {
            knight: whiteKnight,
            bishop: whiteBishop,
            rook: whiteRook,
            queen: whiteQueen
        },
        black: {
            knight: blackKnight,
            bishop: blackBishop,
            rook: blackRook,
            queen: blackQueen
        }
    };

    return (
        <div 
            className="promotion-modal"
            style={{top: `${position.top}px`, left: `${position.left}px`}}
            onClick={(e) => e.stopPropagation()}
        >
            {pieces.map(piece => (
                <img
                    key={piece}
                    className="promotion-option"
                    src={pieceImages[colour][piece]}
                    alt={piece}
                    onClick={() => onSelect(piece)}
                />
            ))}
        </div>
    );
};

export default PromotionModal;