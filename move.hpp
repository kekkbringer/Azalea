#pragma once

/******************************************************************************
 * File containing the 'move' class
 *****************************************************************************/

#include "board.hpp"


/******************************************************************************
 * Move class
 *
 * A move consists of the indicies the piece moves from and to. Additionally
 * contains infomation about whether or not it is a capture, a castling move,
 * a promotion or a double pawn push. For promotion purposes it also contains
 * a instance of 'PieceType'.
 */
class Move {
public:
    int from;
    int to;
    
    bool capture;
    bool castle;
    bool promotion;
    bool doublePawnPush;
    PieceType promopiece;

    Move()
	: from(0)
	, to(0)
	, capture(false)
	, castle(false)
	, promotion(false)
	, doublePawnPush(false)
	, promopiece(PieceType::empty)
	{}

    Move(int f, int t, bool cap, bool cas, bool p, bool b, PieceType pt)
	: from(f)
	, to(t)
	, capture(cap)
	, castle(cas)
	, promotion(p)
	, doublePawnPush(b)
	, promopiece(pt)
	{}
};
