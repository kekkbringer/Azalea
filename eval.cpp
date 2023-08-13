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
			       0,    0,    0,  300,  300,   50,   15,   15,
			       0,    0,    0,   80,   80,   20,    5,    5,
			      10,   10,   10,   10,   10,   10,   10,   10,
			     550,  500,  500,  600,  600,  500,  550,  600,
			       0,    0,    0,    0,    0,    0,    0,    0};

int eval(const GameState& gs) {
    int val = 20; // start with small bonus for the tempo you have

    const Board& b = gs.board;
    
    val += (POP(b.wPawns) - POP(b.bPawns)) * valPawn;
    val += (POP(b.wKnights) - POP(b.bKnights)) * valKnight;
    val += (POP(b.wBishops) - POP(b.bBishops)) * valBishop;
    val += (POP(b.wRooks) - POP(b.bRooks)) * valRook;
    val += (POP(b.wQueens) - POP(b.bQueens)) * valQueen;

    if (!gs.whiteToMove) return -val;
    return val;
}
