import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import ChessBoard from './components/chessboard/chessboard'
import "./index.css"

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <ChessBoard/>
  </StrictMode>
)
