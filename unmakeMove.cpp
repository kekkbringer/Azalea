#include "board.hpp"
#include "move.hpp"

#include <iostream>

void GameState::unmakeMove(UnmakeInfo umi) {
    const int from = umi.from;
    const int to = umi.to;
    const bitb fromMask = (1ULL << from);
    const bitb toMask = (1ULL << to);

    Board& b = this-> board;

    this->epTarget = umi.epTarget;
    this->whiteShort = umi.whiteShort;
    this->whiteLong = umi.whiteLong;
    this->blackShort = umi.blackShort;
    this->blackLong = umi.blackLong;

    // reset moved piece
    if (this->whiteToMove) {
	if (b.bPawns & toMask) {
	    b.bPawns ^= (fromMask | toMask);
	} else if (b.bKnights & toMask) {
	    b.bKnights ^= (fromMask | toMask);
	} else if (b.bBishops & toMask) {
	    b.bBishops ^= (fromMask | toMask);
	} else if (b.bRooks & toMask) {
	    b.bRooks ^= (fromMask | toMask);
	} else if (b.bQueens & toMask) {
	    b.bQueens ^= (fromMask | toMask);
	} else if (b.bKing & toMask) {
	    b.bKing = fromMask;
	}
    } else {
	if (b.wPawns & toMask) {
	    b.wPawns ^= (fromMask | toMask);
	} else if (b.wKnights & toMask) {
	    b.wKnights ^= (fromMask | toMask);
	} else if (b.wBishops & toMask) {
	    b.wBishops ^= (fromMask | toMask);
	} else if (b.wRooks & toMask) {
	    b.wRooks ^= (fromMask | toMask);
	} else if (b.wQueens & toMask) {
	    b.wQueens ^= (fromMask | toMask);
	} else if (b.wKing & toMask) {
	    b.wKing = fromMask;
	}
    }

    // if capture reset the captured piece (non-en-passant caps only)
    if (umi.capturedPiece != pieceType::none and !umi.ep) {
	if (this->whiteToMove) {
	    if (umi.capturedPiece==pieceType::pawn) b.wPawns|=toMask;
	    else if (umi.capturedPiece==pieceType::knight) b.wKnights|=toMask;
	    else if (umi.capturedPiece==pieceType::bishop) b.wBishops|=toMask;
	    else if (umi.capturedPiece==pieceType::rook) b.wRooks|=toMask;
	    else if (umi.capturedPiece==pieceType::queen) b.wQueens|=toMask;
	} else { // black to move
	    if (umi.capturedPiece==pieceType::pawn) b.bPawns|=toMask;
	    else if (umi.capturedPiece==pieceType::knight) b.bKnights|=toMask;
	    else if (umi.capturedPiece==pieceType::bishop) b.bBishops|=toMask;
	    else if (umi.capturedPiece==pieceType::rook) b.bRooks|=toMask;
	    else if (umi.capturedPiece==pieceType::queen) b.bQueens|=toMask;
	}
    }

    // undo en passant capture
    if (umi.ep) {
	const int index = (to%8) + 8*(from/8); // looks stupid, makes sense
	if (this->whiteToMove) { b.wPawns |= (1ULL << index);
	} else { b.bPawns |= (1ULL << index); }
    }

    // promo
    // the piece already moved back but needs to be swapped out for a pawn
    if (umi.promotion) {
	if (this->whiteToMove) {
	    b.bPawns |= fromMask;
	    b.bKnights &= ~fromMask;
	    b.bBishops &= ~fromMask;
	    b.bRooks &= ~fromMask;
	    b.bQueens &= ~fromMask;
	} else { // black to move
	    b.wPawns |= fromMask;
	    b.wKnights &= ~fromMask;
	    b.wBishops &= ~fromMask;
	    b.wRooks &= ~fromMask;
	    b.wQueens &= ~fromMask;
	}
    }
    
    // castle
    if (umi.castle) {
	// we already moved the king, only the rook is left
	if (to ==  1) b.wRooks ^= 0x5ULL; // white kingside
	if (to ==  5) b.wRooks ^= 0x90ULL; // white queenside
	if (to == 57) b.bRooks ^= 0x500000000000000ULL; // black kingside
	if (to == 61) b.bRooks ^= 0x9000000000000000ULL; // black queenside
    }

    b.white = b.wPawns | b.wKnights | b.wBishops
	    | b.wRooks | b.wQueens | b.wKing;
    b.black = b.bPawns | b.bKnights | b.bBishops
	    | b.bRooks | b.bQueens | b.bKing;
    b.occ = b.white | b.black;

    // switch color of course
    this->whiteToMove = !this->whiteToMove;
}
