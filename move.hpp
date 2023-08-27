#pragma once

/******************************************************************************
 * File containing the 'move' class
 *****************************************************************************/


/******************************************************************************
 * Move class
 */
class Move {
public:
    int from;
    int to;

    bool capture;
    bool doublePawnPush;
    bool ep;
    bool castle;
    bool promo;
    char promoPiece;

    Move() : from(0), to(0), capture(false), doublePawnPush(false), ep(false),
		castle(false), promo(false), promoPiece(' ') {}

    Move(int f, int t, bool c, bool d, bool e, bool cas, bool p, char pp)
	: from(f), to(t), capture(c), doublePawnPush(d), ep(e),
		castle(cas), promo(p), promoPiece(pp) {}
};

// auxillary enum
enum class pieceType { none, pawn, knight, bishop, rook, queen };

/******************************************************************************
 * Contains information to unmake a move.
 */
class UnmakeInfo {
public:
    int from;
    int to;
    pieceType capturedPiece;
    bool promotion;
    int epTarget;
    bool whiteShort;
    bool whiteLong;
    bool blackShort;
    bool blackLong;
};
