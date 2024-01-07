#include "board.hpp"
#include "move.hpp"
#include "util.hpp"

#include <iostream>

void GameState::unmakeMove(UnmakeInfo umi, const zobristKeys& zobrist) {
    const int from = umi.from;
    const int to = umi.to;
    const bitb fromMask = (1ULL << from);
    const bitb toMask = (1ULL << to);

    // decrease repetition history counter
    this->repPlyCounter--;

    Board& b = this-> board;

    // if there was an ep target, reset the file
    if (this->epTarget != -1) {
	this->zhash ^= zobrist.epFile[fileOf(this->epTarget)];
    }
    this->epTarget = umi.epTarget;
    // set old ep target if neccessary
    if (umi.epTarget != -1) {
	this->zhash ^= zobrist.epFile[fileOf(umi.epTarget)];
    }

    // check if castling rights actually changed
    if (this->whiteShort != umi.whiteShort) this->zhash ^= zobrist.castling[0];
    if (this->whiteLong  != umi.whiteLong)  this->zhash ^= zobrist.castling[1];
    if (this->blackShort != umi.blackShort) this->zhash ^= zobrist.castling[2];
    if (this->blackLong  != umi.blackLong)  this->zhash ^= zobrist.castling[3];
    // now reset
    this->whiteShort = umi.whiteShort;
    this->whiteLong = umi.whiteLong;
    this->blackShort = umi.blackShort;
    this->blackLong = umi.blackLong;

    // reset moved piece
    if (this->whiteToMove) {
	if (b.bPawns & toMask) {
	    b.bPawns ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][0][from];
	    this->zhash ^= zobrist.pieceSquare[1][0][to];
	} else if (b.bKnights & toMask) {
	    b.bKnights ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][1][from];
	    this->zhash ^= zobrist.pieceSquare[1][1][to];
	} else if (b.bBishops & toMask) {
	    b.bBishops ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][2][from];
	    this->zhash ^= zobrist.pieceSquare[1][2][to];
	} else if (b.bRooks & toMask) {
	    b.bRooks ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][3][from];
	    this->zhash ^= zobrist.pieceSquare[1][3][to];
	} else if (b.bQueens & toMask) {
	    b.bQueens ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[1][4][from];
	    this->zhash ^= zobrist.pieceSquare[1][4][to];
	} else if (b.bKing & toMask) {
	    b.bKing = fromMask;
	    this->zhash ^= zobrist.pieceSquare[1][5][from];
	    this->zhash ^= zobrist.pieceSquare[1][5][to];
	}
    } else {
	if (b.wPawns & toMask) {
	    b.wPawns ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][0][from];
	    this->zhash ^= zobrist.pieceSquare[0][0][to];
	} else if (b.wKnights & toMask) {
	    b.wKnights ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][1][from];
	    this->zhash ^= zobrist.pieceSquare[0][1][to];
	} else if (b.wBishops & toMask) {
	    b.wBishops ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][2][from];
	    this->zhash ^= zobrist.pieceSquare[0][2][to];
	} else if (b.wRooks & toMask) {
	    b.wRooks ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][3][from];
	    this->zhash ^= zobrist.pieceSquare[0][3][to];
	} else if (b.wQueens & toMask) {
	    b.wQueens ^= (fromMask | toMask);
	    this->zhash ^= zobrist.pieceSquare[0][4][from];
	    this->zhash ^= zobrist.pieceSquare[0][4][to];
	} else if (b.wKing & toMask) {
	    b.wKing = fromMask;
	    this->zhash ^= zobrist.pieceSquare[0][5][from];
	    this->zhash ^= zobrist.pieceSquare[0][5][to];
	}
    }

    // if capture reset the captured piece (non-en-passant caps only)
    if (umi.capturedPiece != pieceType::none and !umi.ep) {
	if (this->whiteToMove) {
	    if (umi.capturedPiece==pieceType::pawn) {
		b.wPawns|=toMask;
		this->zhash ^= zobrist.pieceSquare[0][0][to];
	    }
	    else if (umi.capturedPiece==pieceType::knight) {
		b.wKnights|=toMask;
		this->zhash ^= zobrist.pieceSquare[0][1][to];
	    }
	    else if (umi.capturedPiece==pieceType::bishop) {
		b.wBishops|=toMask;
		this->zhash ^= zobrist.pieceSquare[0][2][to];
	    }
	    else if (umi.capturedPiece==pieceType::rook) {
		b.wRooks|=toMask;
		this->zhash ^= zobrist.pieceSquare[0][3][to];
	    }
	    else if (umi.capturedPiece==pieceType::queen) {
		b.wQueens|=toMask;
		this->zhash ^= zobrist.pieceSquare[0][4][to];
	    }
	} else { // black to move
	    if (umi.capturedPiece==pieceType::pawn) {
		b.bPawns|=toMask;
		this->zhash ^= zobrist.pieceSquare[1][0][to];
	    }
	    else if (umi.capturedPiece==pieceType::knight) {
		b.bKnights|=toMask;
		this->zhash ^= zobrist.pieceSquare[1][1][to];
	    }
	    else if (umi.capturedPiece==pieceType::bishop) {
		b.bBishops|=toMask;
		this->zhash ^= zobrist.pieceSquare[1][2][to];
	    }
	    else if (umi.capturedPiece==pieceType::rook) {
		b.bRooks|=toMask;
		this->zhash ^= zobrist.pieceSquare[1][3][to];
	    }
	    else if (umi.capturedPiece==pieceType::queen) {
		b.bQueens|=toMask;
		this->zhash ^= zobrist.pieceSquare[1][4][to];
	    }
	}
    }

    // undo en passant capture
    if (umi.ep) {
	const int index = (to%8) + 8*(from/8); // looks stupid, makes sense
	if (this->whiteToMove) {
	    b.wPawns |= (1ULL << index);
	    this->zhash ^= zobrist.pieceSquare[0][0][index];
	} else {
	    b.bPawns |= (1ULL << index);
	    this->zhash ^= zobrist.pieceSquare[1][0][index];
	}
    }

    // promo
    // the piece already moved back but needs to be swapped out for a pawn
    if (umi.promotion) {
	if (this->whiteToMove) {
	    // set the pawn
	    b.bPawns |= fromMask;
	    this->zhash ^= zobrist.pieceSquare[1][0][from];
	    // reset the piece
	    if (b.bKnights & fromMask) {
		b.bKnights &= ~fromMask;
		this->zhash ^= zobrist.pieceSquare[1][1][from];
	    } else if (b.bBishops & fromMask) {
		b.bBishops &= ~fromMask;
		this->zhash ^= zobrist.pieceSquare[1][2][from];
	    } else if (b.bRooks & fromMask) {
		b.bRooks &= ~fromMask;
		this->zhash ^= zobrist.pieceSquare[1][3][from];
	    } else if (b.bQueens & fromMask) {
		b.bQueens &= ~fromMask;
		this->zhash ^= zobrist.pieceSquare[1][4][from];
	    }
	} else { // black to move
	    // set the pawn
	    b.wPawns |= fromMask;
	    this->zhash ^= zobrist.pieceSquare[0][0][from];
	    // reset the piece
	    if (b.wKnights & fromMask) {
		b.wKnights &= ~fromMask;
		this->zhash ^= zobrist.pieceSquare[0][1][from];
	    } else if (b.wBishops & fromMask) {
		b.wBishops &= ~fromMask;
		this->zhash ^= zobrist.pieceSquare[0][2][from];
	    } else if (b.wRooks & fromMask) {
		b.wRooks &= ~fromMask;
		this->zhash ^= zobrist.pieceSquare[0][3][from];
	    } else if (b.wQueens & fromMask) {
		b.wQueens &= ~fromMask;
		this->zhash ^= zobrist.pieceSquare[0][4][from];
	    }
	}
    }
    
    // castle
    if (umi.castle) {
	// we already moved the king, only the rook is left
	if (to ==  1) {
	    b.wRooks ^= 0x5ULL; // white kingside
	    this->zhash ^= zobrist.pieceSquare[0][3][2];
	    this->zhash ^= zobrist.pieceSquare[0][3][0];
	}
	if (to ==  5) {
	    b.wRooks ^= 0x90ULL; // white queenside
	    this->zhash ^= zobrist.pieceSquare[0][3][4];
	    this->zhash ^= zobrist.pieceSquare[0][3][7];
	}
	if (to == 57) {
	    b.bRooks ^= 0x500000000000000ULL; // black kingside
	    this->zhash ^= zobrist.pieceSquare[1][3][58];
	    this->zhash ^= zobrist.pieceSquare[1][3][56];
	}
	if (to == 61) {
	    b.bRooks ^= 0x9000000000000000ULL; // black queenside
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
}
