#pragma once

#include "params.hpp"

/******************************************************************************
 * This file contains the board representation as well as some auxiliary
 * definitions
 *****************************************************************************/


/******************************************************************************
 * Board class
 *
 * Using bitboards...
 */
class Board {
public:
    // white pieces
    bitb wPawns;
    bitb wKnights;
    bitb wBishops;
    bitb wRooks;
    bitb wQueens;
    bitb wKing;

    // black pieces
    bitb bPawns;
    bitb bKnights;
    bitb bBishops;
    bitb bRooks;
    bitb bQueens;
    bitb bKing;

    // color and occupancy
    bitb white;
    bitb black;
    bitb occ;

    Board(bitb wp, bitb wk, bitb wb, bitb wr, bitb wq, bitb wk,
	  bitb bp, bitb bk, bitb bb, bitb br, bitb bq, bitb bk) :
	wPawns(wp), wKnights(wk), wBishops(wb), wRooks(wr), wQueens(wq),
	bPawns(bp), bKnights(bk), bBishops(bb), bRooks(br), bQueens(bq),
	wKing(wk), bKing(bk),
	white(wp|wk|wb|wr|wq|wk), black(bp|bk|bb|br|bq|bk), occ(white|black)
	{}
};
