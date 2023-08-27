#include "board.hpp"
#include "move.hpp"

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
	}
    }

    // if capture reset the captured piece
    if (umi.capturedPiece != pieceType::none) {
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

    // TODO promo
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
    
    // TODO castle komplett (UnmakeInfo muss noch ein flag bekommen, sonst wirds hart)

    b.white = b.wPawns | b.wKnights | b.wBishops
	    | b.wRooks | b.wQueens | b.wKing;
    b.black = b.bPawns | b.bKnights | b.bBishops
	    | b.bRooks | b.bQueens | b.bKing;
    b.occ = b.white | b.black;

    // switch color of course
    this->whiteToMove = !this->whiteToMove;
}
