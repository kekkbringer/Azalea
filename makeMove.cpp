#include "params.hpp"
#include "board.hpp"
#include "move.hpp"
#include "util.hpp"

UnmakeInfo GameState::makeMove(const Move& m, const zobristKeys& zobrist) {
    const int from = m.from;
    const int to = m.to;
    const bitb fromMask = (1ULL << from);
    const bitb toMask = (1ULL << to);

    Board& b = this->board;

    // prepare info to unmake move
    UnmakeInfo umi;
    umi.from = from;
    umi.to = to;
    umi.capturedPiece = pieceType::none;
    umi.promotion = m.promo;
    umi.castle = m.castle;
    umi.ep = m.ep;
    umi.epTarget = this->epTarget;
    umi.whiteShort = this->whiteShort;
    umi.whiteLong = this->whiteLong;
    umi.blackShort = this->blackShort;
    umi.blackLong = this->blackLong;

    if (m.capture) {
	if (toMask & (b.wPawns | b.bPawns)) {
	    umi.capturedPiece = pieceType::pawn;
	    if (not m.ep) {
		if (this->whiteToMove) {
		    this->zhash ^= zobrist.pieceSquare[1][0][to];
		} else {
		    this->zhash ^= zobrist.pieceSquare[0][0][to];
		}
	    }
	} else if (toMask & (b.wKnights | b.bKnights)) {
	    umi.capturedPiece = pieceType::knight;
	    if (this->whiteToMove) {
		this->zhash ^= zobrist.pieceSquare[1][1][to];
	    } else {
		this->zhash ^= zobrist.pieceSquare[0][1][to];
	    }
	} else if (toMask & (b.wBishops | b.bBishops)) {
	    umi.capturedPiece = pieceType::bishop;
	    if (this->whiteToMove) {
		this->zhash ^= zobrist.pieceSquare[1][2][to];
	    } else {
		this->zhash ^= zobrist.pieceSquare[0][2][to];
	    }
	} else if (toMask & (b.wRooks | b.bRooks)) {
	    umi.capturedPiece = pieceType::rook;
	    if (this->whiteToMove) {
		this->zhash ^= zobrist.pieceSquare[1][3][to];
	    } else {
		this->zhash ^= zobrist.pieceSquare[0][3][to];
	    }
	} else if (toMask & (b.wQueens | b.bQueens)) {
	    if (this->whiteToMove) {
		this->zhash ^= zobrist.pieceSquare[1][4][to];
	    } else {
		this->zhash ^= zobrist.pieceSquare[0][4][to];
	    }
	    umi.capturedPiece = pieceType::queen;
	}
    }

    // if en passant capture, remove pawn
    if (m.ep) {
	const int index = (to%8) + 8*(from/8); // looks stupid, makes sense
	b.wPawns &= ~(1ULL << index);
	b.bPawns &= ~(1ULL << index);
	if (this->whiteToMove) {
	    this->zhash ^= zobrist.pieceSquare[1][0][index];
	} else {
	    this->zhash ^= zobrist.pieceSquare[0][0][index];
	}
    }

    // reset ep target
    if (epTarget != -1) {
	this->zhash ^= zobrist.epFile[fileOf(this->epTarget)];
    }
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

    // check if castling rights changed for Zobrist hash
    if (this->whiteShort != umi.whiteShort) this->zhash ^= zobrist.castling[0];
    if (this->whiteLong  != umi.whiteLong)  this->zhash ^= zobrist.castling[1];
    if (this->blackShort != umi.blackShort) this->zhash ^= zobrist.castling[2];
    if (this->blackLong  != umi.blackLong)  this->zhash ^= zobrist.castling[3];

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
	    this->zhash ^= zobrist.pieceSquare[0][0][from];
	    this->zhash ^= zobrist.pieceSquare[0][0][to];
	} else if (b.wKnights & fromMask) {
	    b.wKnights ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][1][from];
	    this->zhash ^= zobrist.pieceSquare[0][1][to];
	} else if (b.wBishops & fromMask) {
	    b.wBishops ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][2][from];
	    this->zhash ^= zobrist.pieceSquare[0][2][to];
	} else if (b.wRooks & fromMask) {
	    b.wRooks ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][3][from];
	    this->zhash ^= zobrist.pieceSquare[0][3][to];
	} else if (b.wQueens & fromMask) {
	    b.wQueens ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][4][from];
	    this->zhash ^= zobrist.pieceSquare[0][4][to];
	} else if (b.wKing & fromMask) {
	    b.wKing = toMask;
	    this->zhash ^= zobrist.pieceSquare[0][5][from];
	    this->zhash ^= zobrist.pieceSquare[0][5][to];
	}
    } else { // black moves
	// get moving piece and move it
	if (b.bPawns & fromMask) {
	    b.bPawns ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][0][from];
	    this->zhash ^= zobrist.pieceSquare[1][0][to];
	} else if (b.bKnights & fromMask) {
	    b.bKnights ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][1][from];
	    this->zhash ^= zobrist.pieceSquare[1][1][to];
	} else if (b.bBishops & fromMask) {
	    b.bBishops ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][2][from];
	    this->zhash ^= zobrist.pieceSquare[1][2][to];
	} else if (b.bRooks & fromMask) {
	    b.bRooks ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][3][from];
	    this->zhash ^= zobrist.pieceSquare[1][3][to];
	} else if (b.bQueens & fromMask) {
	    b.bQueens ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][4][from];
	    this->zhash ^= zobrist.pieceSquare[1][4][to];
	} else if (b.bKing & fromMask) {
	    b.bKing = toMask;
	    this->zhash ^= zobrist.pieceSquare[1][5][from];
	    this->zhash ^= zobrist.pieceSquare[1][5][to];
	}
    }

    // double pawn push -> set ep square
    if (m.doublePawnPush) {
	this->epTarget = (from + to)/2;
	this->zhash ^= zobrist.epFile[fileOf(this->epTarget)];
    }

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
		    this->zhash ^= zobrist.pieceSquare[0][0][to];
		    this->zhash ^= zobrist.pieceSquare[0][4][to];
		    break;
		case 'n':
		    b.wKnights ^= toMask;
		    this->zhash ^= zobrist.pieceSquare[0][0][to];
		    this->zhash ^= zobrist.pieceSquare[0][1][to];
		    break;
		case 'b':
		    b.wBishops ^= toMask;
		    this->zhash ^= zobrist.pieceSquare[0][0][to];
		    this->zhash ^= zobrist.pieceSquare[0][2][to];
		    break;
		case 'r':
		    b.wRooks ^= toMask;
		    this->zhash ^= zobrist.pieceSquare[0][0][to];
		    this->zhash ^= zobrist.pieceSquare[0][3][to];
		    break;
	    }
	} else { // black
	    b.bPawns ^= toMask;
	    switch (m.promoPiece) {
		case 'q':
		    b.bQueens ^= toMask;
		    this->zhash ^= zobrist.pieceSquare[1][0][to];
		    this->zhash ^= zobrist.pieceSquare[1][4][to];
		    break;
		case 'n':
		    b.bKnights ^= toMask;
		    this->zhash ^= zobrist.pieceSquare[1][0][to];
		    this->zhash ^= zobrist.pieceSquare[1][1][to];
		    break;
		case 'b':
		    b.bBishops ^= toMask;
		    this->zhash ^= zobrist.pieceSquare[1][0][to];
		    this->zhash ^= zobrist.pieceSquare[1][2][to];
		    break;
		case 'r':
		    b.bRooks ^= toMask;
		    this->zhash ^= zobrist.pieceSquare[1][0][to];
		    this->zhash ^= zobrist.pieceSquare[1][3][to];
		    break;
	    }
	}
    }

    // castling
    if (m.castle) {
	// we already moved the king, only the rook is left
	if (to ==  1) { // white kingside
	    b.wRooks ^= 0x5ULL;
	    this->zhash ^= zobrist.pieceSquare[0][3][2];
	    this->zhash ^= zobrist.pieceSquare[0][3][0];
	}
	if (to ==  5) { // white queenside
	    b.wRooks ^= 0x90ULL;
	    this->zhash ^= zobrist.pieceSquare[0][3][4];
	    this->zhash ^= zobrist.pieceSquare[0][3][7];
	}
	if (to == 57) { // black kingside
	    b.bRooks ^= 0x500000000000000ULL;
	    this->zhash ^= zobrist.pieceSquare[1][3][58];
	    this->zhash ^= zobrist.pieceSquare[1][3][56];
	}
	if (to == 61) { // black queenside
	    b.bRooks ^= 0x9000000000000000ULL;
	    this->zhash ^= zobrist.pieceSquare[1][3][60];
	    this->zhash ^= zobrist.pieceSquare[1][3][63];
	}
    }

    b.white = b.wPawns | b.wKnights | b.wBishops
	    | b.wRooks | b.wQueens | b.wKing;
    b.black = b.bPawns | b.bKnights | b.bBishops
	    | b.bRooks | b.bQueens | b.bKing;
    b.occ = b.white | b.black;

    // switch color of course
    this->whiteToMove = !this->whiteToMove;
    this->zhash ^= zobrist.sideToMove;

    return umi;
}
