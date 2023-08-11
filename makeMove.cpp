#include "params.hpp"
#include "board.hpp"
#include "move.hpp"

void GameState::makeMove(const Move& m) {
    const int from = m.from;
    const int to = m.to;
    const bitb fromMask = (1ULL << from);
    const bitb toMask = (1ULL << to);

    Board& b = this->board;

    // reset ep target
    this->epTarget = -1;

    // reset castle rights if anything moves from a1, e1, h1, a8, e8, h8
    // original rook square first
    if (from ==  7) this->whiteLong  = false;
    if (from ==  0) this->whiteShort = false;
    if (from == 63) this->blackLong  = false;
    if (from == 56) this->blackShort = false;
    // original king squares
    if (from ==  3) {this->whiteShort = false; this->whiteLong = false;}
    if (from == 59) {this->blackShort = false; this->blackLong = false;}
    
    // also reset castle rights if anything moves to a1, h1, a8, h8
    if (to ==  7) this->whiteLong  = false;
    if (to ==  0) this->whiteShort = false;
    if (to == 63) this->blackLong  = false;
    if (to == 56) this->blackShort = false;

    // first zero the to-square everywhere (king not needed, cant be captured)
    // for an ep capture another square needs to be cleared as well (later)
    b.wPawns &= ~toMask;
    b.wKnights &= ~toMask;
    b.wBishops &= ~toMask;
    b.wRooks &= ~toMask;
    b.wQueens &= ~toMask;
    b.bPawns &= ~toMask;
    b.bKnights &= ~toMask;
    b.bBishops &= ~toMask;
    b.bRooks &= ~toMask;
    b.bQueens &= ~toMask;
    
    if (this->whiteToMove) {
	// get moving piece and move it
	if (b.wPawns & fromMask) {
	    b.wPawns ^= (fromMask | toMask);
	} else if (b.wKnights & fromMask) {
	    b.wKnights ^= (fromMask | toMask);
	} else if (b.wBishops & fromMask) {
	    b.wBishops ^= (fromMask | toMask);
	} else if (b.wRooks & fromMask) {
	    b.wRooks ^= (fromMask | toMask);
	} else if (b.wQueens & fromMask) {
	    b.wQueens ^= (fromMask | toMask);
	} else if (b.wKing & fromMask) {
	    b.wKing = toMask;
	}
    } else { // black moves
	// get moving piece and move it
	if (b.bPawns & fromMask) {
	    b.bPawns ^= (fromMask | toMask);
	} else if (b.bKnights & fromMask) {
	    b.bKnights ^= (fromMask | toMask);
	} else if (b.bBishops & fromMask) {
	    b.bBishops ^= (fromMask | toMask);
	} else if (b.bRooks & fromMask) {
	    b.bRooks ^= (fromMask | toMask);
	} else if (b.bQueens & fromMask) {
	    b.bQueens ^= (fromMask | toMask);
	} else if (b.bKing & fromMask) {
	    b.bKing = toMask;
	}
    }

    // double pawn push -> set ep square
    if (m.doublePawnPush) this->epTarget = (from + to)/2;

    // promotions
    if (m.promo) {
	// we already move the pawn, now the pawn needs to be replaced with the
	// respective piece it's promoting to (can obviously be done more
	// efficient)
	if (this->whiteToMove) {
	    b.wPawns ^= toMask;
	    switch (m.promoPiece) {
		case 'q':
		    b.wQueens ^= toMask;
		    break;
		case 'n':
		    b.wKnights ^= toMask;
		    break;
		case 'b':
		    b.wBishops ^= toMask;
		    break;
		case 'r':
		    b.wRooks ^= toMask;
		    break;
	    }
	} else { // black
	    b.bPawns ^= toMask;
	    switch (m.promoPiece) {
		case 'q':
		    b.bQueens ^= toMask;
		    break;
		case 'n':
		    b.bKnights ^= toMask;
		    break;
		case 'b':
		    b.bBishops ^= toMask;
		    break;
		case 'r':
		    b.bRooks ^= toMask;
		    break;
	    }
	}
    }

    // castling
    if (m.castle) {
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
