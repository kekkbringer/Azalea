#pragma once

#include "params.hpp"
#include "move.hpp"

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

    Board() :
	wPawns(0), wKnights(0), wBishops(0), wRooks(0), wQueens(0), wKing(0),
	bPawns(0), bKnights(0), bBishops(0), bRooks(0), bQueens(0), bKing(0),
	white(0), black(0), occ(0)
	{}

    Board(bitb wp, bitb wn, bitb wb, bitb wr, bitb wq, bitb wk,
	  bitb bp, bitb bn, bitb bb, bitb br, bitb bq, bitb bk) :
	wPawns(wp), wKnights(wn), wBishops(wb),
	wRooks(wr), wQueens(wq), wKing(wk),
	bPawns(bp), bKnights(bn), bBishops(bb),
	bRooks(br), bQueens(bq), bKing(bk),
	white(wp|wk|wb|wr|wq|wk), black(bp|bk|bb|br|bq|bk), occ(white|black)
	{}
};

/******************************************************************************
 * Game state class
 *
 * Contains the board state and additional stuff like castling rights.
 */
class GameState {
public:
    Board board;
    bool whiteToMove;
    bool whiteShort;
    bool whiteLong;
    bool blackShort;
    bool blackLong;
    int halfmoveClock;
    int epTarget;

    void makeMove(const Move& m);
};

// perft function with bulk counting, no div
unsigned long long int perft(const GameState& gs, int depth);
