#include "board.hpp"
#include "params.hpp"

#include <iostream>

constexpr double F = 0.05;
// Score of Azalea 1.1.0A vs Azalea 1.1.0B: 342 - 443 - 201  [0.449] 986
// Score of Azalea 1.1.0a vs Azalea 1.1.0tuned: 372 - 404 - 210  [0.484] 986
//
// Score of Azalea 1.1.0A vs Azalea 1.1.0B: 395 - 376 - 215  [0.510] 986
//

constexpr int earlyValPawn   = 1000 - 124*F + 28*F;
constexpr int earlyValKnight = 3000 +   4*F +  5*F;
constexpr int earlyValBishop = 3250 +  90*F + 15*F;
constexpr int earlyValRook   = 5000 + 100*F - 20*F;
constexpr int earlyValQueen  = 9000 +  89*F - 15*F;

constexpr int lateValPawn   = 1300 -  54*F + 13*F;
constexpr int lateValKnight = 2650 + 116*F - 23*F;
constexpr int lateValBishop = 3100 -  58*F +  8*F;
constexpr int lateValRook   = 6000 +  96*F + 51*F;
constexpr int lateValQueen  = 9000 -   8*F + 19*F;

constexpr int pawnPhase   = 0;
constexpr int knightPhase = 200;
constexpr int bishopPhase = 200;
constexpr int rookPhase   = 400;
constexpr int queenPhase  = 600;

constexpr int startPhase = 16 * pawnPhase
			  + 4 * knightPhase
			  + 4 * bishopPhase
			  + 4 * rookPhase
			  + 2 * queenPhase;

constexpr int earlyPawnSq[64] = {   0,    0,    0,    0,    0,    0,    0,    0,
			     100,  100,  100,  -60,  -60,  -20,  -20,  -20,
			      10,   10,  -80,  -10,  -10,   10,    0,    0,
			       8,    2,    3,  100,  100,   50,   15,   15,
			       9,    9,    9,   40,   40,   20,    5,    5,
			      10,   10,   10,   10,   10,   10,   10,   10,
			     300,  300,  300,  300,  300,  300,  300,  300,
			       0,    0,    0,    0,    0,    0,    0,    0};

constexpr int latePawnSq[64] = {  0,    0,    0,    0,    0,    0,    0,    0,
			        100,  100,  100,  100,  100,  100,  100,  100,
			        200,  200,  200,  200,  200,  200,  200,  200,
			        400,  400,  400,  400,  400,  400,  400,  400,
			        600,  600,  600,  600,  600,  600,  600,  600,
			        800,  800,  800,  800,  800,  800,  800,  800,
			       1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
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

constexpr int earlyKingSq[64] = {  500,  700,    0,    0, -300,  400,  420,  400,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0,
			        0,    0,    0,    0,    0,    0,    0,    0};

constexpr int lateKingSq[64] = {-1000, -200, -120, -100, -100, -120, -200,-1000,
			       -500, -100,    0,    0,    0,    0, -100, -500,
			       -300,    0,   50,   40,   40,   50,    0, -300,
			       -300,   10,   15,   45,   45,   15,   10, -300,
			       -300,    8,   15,   45,   45,   15,    8, -300,
			       -300,    0,   10,   30,   30,   10,    0, -300,
			       -500, -100,    0,    0,    0,    0, -100, -500,
			      -1000, -500, -300, -250, -250, -300, -500,-1000};

constexpr int rookSq[64] = {    0,    0,  220,  250,  250,  220,    0,    0,
			        0,    0,    6,    7,    7,    6,    0,    0,
			        0,    6,    7,    8,    8,    7,    6,    0,
			        6,    7,    8,    9,    9,    8,    7,    6,
			        6,    7,    8,    9,    9,    8,    7,    6,
			        0,    6,    7,    8,    8,    7,    6,    0,
			      250,  250,  250,  250,  250,  250,  250,  250,
			        0,    0,    0,    0,    0,    0,    0,    0};

constexpr int lateQueenSq[64] = {    0,    0,   30,   50,   50,   30,   0,   0,
			             0,   30,   50,  150,  150,   50,  30,   0,
			            30,   50,  150,  200,  200,  150,  50,  30,
			            50,  150,  200,  200,  200,  200, 150,  50,
			            50,  150,  200,  200,  200,  200, 150,  50,
			            30,   50,  150,  200,  200,  150,  50,  30,
			             0,   30,   50,  150,  150,   50,  30,   0,
			             0,    0,   30,   50,   50,   30,   0,   0};

template <bool lprint>
int eval(const GameState& gs) {
    int earlyEval = 100; // start with small bonus for the tempo you have
    int lateEval = 0;

    int earlyMatEval = 0;
    int earlyPosEval = 0;
    int lateMatEval = 0;
    int latePosEval = 0;

    const Board& b = gs.board;

    const int nwp = POP(b.wPawns);
    const int nwn = POP(b.wKnights);
    const int nwb = POP(b.wBishops);
    const int nwr = POP(b.wRooks);
    const int nwq = POP(b.wQueens);
    const int nbp = POP(b.bPawns);
    const int nbn = POP(b.bKnights);
    const int nbb = POP(b.bBishops);
    const int nbr = POP(b.bRooks);
    const int nbq = POP(b.bQueens);

    earlyMatEval += (nwp - nbp) * earlyValPawn;
    earlyMatEval += (nwn - nbn) * earlyValKnight;
    earlyMatEval += (nwb - nbb) * earlyValBishop;
    earlyMatEval += (nwr - nbr) * earlyValRook;
    earlyMatEval += (nwq - nbq) * earlyValQueen;

    lateMatEval += (nwp - nbp) * lateValPawn;
    lateMatEval += (nwn - nbn) * lateValKnight;
    lateMatEval += (nwb - nbb) * lateValBishop;
    lateMatEval += (nwr - nbr) * lateValRook;
    lateMatEval += (nwq - nbq) * lateValQueen;

    earlyEval += earlyMatEval;
    lateEval += lateMatEval;

    // calculate game phase
    const int phase = (nwp + nbp) * pawnPhase
		    + (nwn + nbn) * knightPhase
		    + (nwb + nbb) * bishopPhase
		    + (nwr + nbr) * rookPhase
		    + (nwq + nbq) * queenPhase;

    // square specific pawn value
    bitb wp = b.wPawns;
    while (wp) {
	const int index = BSF(wp); wp &= wp - 1;
	earlyPosEval += earlyPawnSq[index];
	latePosEval += latePawnSq[index];
    }
    bitb bp = b.bPawns;
    while (bp) {
	const int index = BSF(bp); bp &= bp - 1;
	earlyPosEval -= earlyPawnSq[index^56];
	latePosEval -= latePawnSq[index^56];
    }

    // square specific knight value
    bitb wn = b.wKnights;
    while (wn) {
	const int index = BSF(wn); wn &= wn - 1;
	earlyPosEval += knightSq[index];
	latePosEval += knightSq[index];
    }
    bitb bn = b.bKnights;
    while (bn) {
	const int index = BSF(bn); bn &= bn - 1;
	earlyPosEval -= knightSq[index^56];
	latePosEval -= knightSq[index^56];
    }

    // square specific bishop value
    bitb wb = b.wBishops;
    while (wb) {
	const int index = BSF(wb); wb &= wb - 1;
	earlyPosEval += bishopSq[index];
	latePosEval += bishopSq[index];
    }
    bitb bb = b.bBishops;
    while (bb) {
	const int index = BSF(bb); bb &= bb - 1;
	earlyPosEval -= bishopSq[index^56];
	latePosEval -= bishopSq[index^56];
    }

    // square specific rook value
    bitb wr = b.wRooks;
    while (wr) {
	const int index = BSF(wr); wr &= wr - 1;
	earlyPosEval += rookSq[index];
	latePosEval += rookSq[index];
    }
    bitb br = b.bRooks;
    while (br) {
	const int index = BSF(br); br &= br - 1;
	earlyPosEval -= rookSq[index^56];
	latePosEval -= rookSq[index^56];
    }

    // square specific queen value
    bitb wq = b.wQueens;
    while (wq) {
	const int index = BSF(wq); wq &= wq - 1;
	latePosEval += lateQueenSq[index];
    }
    bitb bq = b.bQueens;
    while (bq) {
	const int index = BSF(bq); bq &= bq - 1;
	latePosEval -= lateQueenSq[index^56];
    }

    // square specific king value
    bitb wk = b.wKing;
    earlyPosEval += earlyKingSq[BSF(wk)];
    latePosEval += lateKingSq[BSF(wk)];
    bitb bk = b.bKing;
    earlyPosEval -= earlyKingSq[BSF(bk)^56];
    latePosEval -= lateKingSq[BSF(bk)^56];

    earlyEval += earlyPosEval;
    lateEval += latePosEval;

    const int val = (phase * earlyEval)/startPhase
		+ ((startPhase - phase) * lateEval)/startPhase;
    if (lprint) {
        std::cout << " :: EVALUATION BREAKDOWN\n";
        std::cout << "early phase weight: "
        			<< (phase*100)/startPhase << "%\n";
        std::cout << "end game weight:    "
        			<< (startPhase-phase)*100/startPhase << "%\n";
	std::cout << "early game eval: " << earlyEval << "\n";
	std::cout << "end game eval: " << lateEval << "\n";
	std::cout << "early game material eval: " << earlyMatEval << "\n";
	std::cout << "end game material eval:   " << lateMatEval << "\n";
	std::cout << "early game square eval: " << earlyPosEval << "\n";
	std::cout << "end game square eval:   " << latePosEval << "\n";
    }

    if (!gs.whiteToMove) return -val;
    return val;
}

