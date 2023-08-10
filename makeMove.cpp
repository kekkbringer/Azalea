#include "params.hpp"
#include "board.hpp"
#include "move.hpp"

void GameState::makeMove(const Move& m) {
    const int from = m.from;
    const int to = m.to;
    const bitb fromMask = (1ULL << from);
    const bitb toMask = (1ULL << to);

    Board& b = this->board;

    // first zero to to-square everywhere (king not needed, cant be captured)
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

    b.white = b.wPawns | b.wKnights | b.wBishops
	    | b.wRooks | b.wQueens | b.wKing;
    b.black = b.bPawns | b.bKnights | b.bBishops
	    | b.bRooks | b.bQueens | b.bKing;
    b.occ = b.white | b.black;

    // switch color of course
    this->whiteToMove = !this->whiteToMove;
}
