#include "eval.hpp"

#include "board.hpp"
#include "params.hpp"

#include <iostream> // TODO delete

constexpr int valPawn   = 1000;
constexpr int valKnight = 3000;
constexpr int valBishop = 3250;
constexpr int valRook   = 5000;
constexpr int valQueen  = 9000;

constexpr int pawnSq[64] = {   0,    0,    0,    0,    0,    0,    0,    0,
			     300,  300,  300, -200, -200,  -50,  -50,  -50,
			      10,   10, -200,  -10,  -10,   10,    0,    0,
			       8,    2,    3,  300,  300,   50,   15,   15,
			       9,    9,    9,   80,   80,   20,    5,    5,
			      10,   10,   10,   10,   10,   10,   10,   10,
			     550,  500,  500,  600,  600,  500,  550,  600,
			       0,    0,    0,    0,    0,    0,    0,    0};

constexpr int knightSq[64] = {-1000, -200, -120, -100, -100, -120, -200,-1000,
			       -500, -100,    0,    0,    0,    0, -100, -500,
			       -300,    0,   50,   40,   40,   50,    0, -300,
			       -300,   10,   15,   45,   45,   15,   10, -300,
			       -300,    8,   15,   45,   45,   15,    8, -300,
			       -300,    0,   10,   30,   30,   10,    0, -300,
			       -500, -100,    0,    0,    0,    0, -100, -500,
			      -1000, -500, -300, -250, -250, -300, -500,-1000};
constexpr int bishopSq[64] = {  -11,  -10,  -12,  -12,  -12,  -12,  -10,  -11,
				-10,   20,    2,    5,    5,    7,   20,  -10,
				  0,    0,    1,    3,    3,    1,    0,    0,
				  0,    1,   15,   10,   10,   15,    1,    0,
				  0,    1,    9,    8,    8,    9,    1,    0,
				  0,    0,    1,    3,    3,    1,    0,    0,
				-10,    1,    0,    0,    0,    0,    1,  -10,
				-11,  -10,   -5,   -5,   -5,   -5,  -10,  -11};

constexpr int kingSq[64] = {  500,  700,    0,    0, -300,  400,  420,  400,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0};

constexpr int rookSq[64] = {    0,    0,  220,  250,  250,  220,    0,    0,
			        0,    0,    6,    7,    7,    6,    0,    0,
			        0,    6,    7,    8,    8,    7,    6,    0,
			        6,    7,    8,    9,    9,    8,    7,    6,
			        6,    7,    8,    9,    9,    8,    7,    6,
			        0,    6,    7,    8,    8,    7,    6,    0,
			      250,  250,  250,  250,  250,  250,  250,  250,
			        0,    0,    0,    0,    0,    0,    0,    0};


int eval(const GameState& gs) {
    int val = 20; // start with small bonus for the tempo you have

    const Board& b = gs.board;
    
    val += (POP(b.wPawns) - POP(b.bPawns)) * valPawn;
    val += (POP(b.wKnights) - POP(b.bKnights)) * valKnight;
    val += (POP(b.wBishops) - POP(b.bBishops)) * valBishop;
    val += (POP(b.wRooks) - POP(b.bRooks)) * valRook;
    val += (POP(b.wQueens) - POP(b.bQueens)) * valQueen;

    // square specific pawn value
    bitb wp = b.wPawns;
    while (wp) {
	const int index = BSF(wp); wp &= wp - 1;
	val += pawnSq[index];
    }
    bitb bp = b.bPawns;
    while (bp) {
	const int index = BSF(bp); bp &= bp - 1;
	val -= pawnSq[index^56];
    }

    // square specific knight value
    bitb wn = b.wKnights;
    while (wn) {
	const int index = BSF(wn); wn &= wn - 1;
	val += knightSq[index];
    }
    bitb bn = b.bKnights;
    while (bn) {
	const int index = BSF(bn); bn &= bn - 1;
	val -= knightSq[index^56];
    }

    // square specific bishop value
    bitb wb = b.wBishops;
    while (wb) {
	const int index = BSF(wb); wb &= wb - 1;
	val += bishopSq[index];
    }
    bitb bb = b.bBishops;
    while (bb) {
	const int index = BSF(bb); bb &= bb - 1;
	val -= bishopSq[index^56];
    }

    // square specific rook value
    bitb wr = b.wRooks;
    while (wr) {
	const int index = BSF(wr); wr &= wr - 1;
	val += rookSq[index];
    }
    bitb br = b.bRooks;
    while (br) {
	const int index = BSF(br); br &= br - 1;
	val -= rookSq[index^56];
    }

    // square specific king value
    bitb wk = b.wKing; val += kingSq[BSF(wk)];
    bitb bk = b.bKing; val -= kingSq[BSF(bk)^56];

    if (!gs.whiteToMove) return -val;
    return val;
}
