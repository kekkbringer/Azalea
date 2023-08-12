#include "eval.hpp"

#include "board.hpp"
#include "params.hpp"

#include <iostream> // TODO delete

constexpr int valPawn   = 1000;
constexpr int valKnight = 3000;
constexpr int valBishop = 3250;
constexpr int valRook   = 5000;
constexpr int valQueen  = 9000;

int eval(const GameState& gs) {
    int val = 0;

    const Board& b = gs.board;
    
    val += (POP(b.wPawns) - POP(b.bPawns)) * valPawn;
    val += (POP(b.wKnights) - POP(b.bKnights)) * valKnight;
    val += (POP(b.wBishops) - POP(b.bBishops)) * valBishop;
    val += (POP(b.wRooks) - POP(b.bRooks)) * valRook;
    val += (POP(b.wQueens) - POP(b.bQueens)) * valQueen;

    if (!gs.whiteToMove) return -val;
    return val;
}
