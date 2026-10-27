#pragma once

#include "board.hpp"
#include "engine.hpp"

#include <cstdlib> // std::abs

namespace azalea {

constexpr int SCORE_INF  = 1048576;
constexpr int SCORE_MATE = 1048575;
constexpr int MATE_BOUND = SCORE_MATE - MAX_PLY;

constexpr bool is_mate_score(int s) { return std::abs(s) >= MATE_BOUND; }

constexpr int TEMPO_EVAL = 10;

constexpr int EARLY_PAWN_EVAL   = 100;
constexpr int EARLY_KNIGHT_EVAL = 300;
constexpr int EARLY_BISHOP_EVAL = 310;
constexpr int EARLY_ROOK_EVAL   = 450;
constexpr int EARLY_QUEEN_EVAL  = 900;

constexpr int LATE_PAWN_EVAL   = 125;
constexpr int LATE_KNIGHT_EVAL = 300;
constexpr int LATE_BISHOP_EVAL = 350;
constexpr int LATE_ROOK_EVAL   = 520;
constexpr int LATE_QUEEN_EVAL  = 900;

constexpr int PAWN_MATERIAL   =  1;
constexpr int KNIGHT_MATERIAL =  3;
constexpr int BISHOP_MATERIAL =  3;
constexpr int ROOK_MATERIAL   =  4;
constexpr int QUEEN_MATERIAL  =  5;

constexpr int EARLY_PAWN_SQ[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, -2, -5, -5, 10, 10, 10,
     0,  5,  5,  5,  5,  5,  5,  9,
     0,  5, 15, 20, 20, 15,  5,  0,
     0,  5, 10, 20, 20, 10,  5,  0,
     0,  5,  5,  5,  5,  5,  5,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr int LATE_PAWN_SQ[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, 10, 10, 10,  0,  0,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
    10, 20, 20, 20, 20, 20, 20, 10,
    60, 70, 70, 70, 70, 70, 70, 60,
    80, 90, 90, 90, 90, 90, 90, 80,
     0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr int EARLY_KNIGHT_SQ[64] = {
    -9,  0,  0,  0,  0,  0,  0, -9,
     0, 10, 10, 10, 10, 10, 10,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
     0, 10, 10, 10, 10, 10, 10,  0,
    -9,  0,  0,  0,  0,  0,  0, -9,
};

constexpr int LATE_KNIGHT_SQ[64] = {
    -9,  0,  0,  0,  0,  0,  0, -9,
     0, 10, 10, 10, 10, 10, 10,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
     0, 10, 20, 20, 20, 20, 10,  0,
     0, 10, 10, 10, 10, 10, 10,  0,
    -9,  0,  0,  0,  0,  0,  0, -9,
};

constexpr int EARLY_BISHOP_SQ[64] = {
     0,  0, -5,  0,  0, -5,  0,  0,
     0, 20,  0,  0,  0,  0, 20,  0,
     0,  0, 10, 10, 10, 10,  0,  0,
     0,  0, 20, 10, 10, 20,  0,  0,
     0,  0, 10, 10, 10, 10,  0,  0,
     0,  0, 10, 10, 10, 10,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr int LATE_BISHOP_SQ[64] = {
     0,  0, -5,  0,  0, -5,  0,  0,
     0, 20,  0,  0,  0,  0, 20,  0,
     0,  0, 10, 10, 10, 10,  0,  0,
     0,  0, 20, 10, 10, 20,  0,  0,
     0,  0, 10, 10, 10, 10,  0,  0,
     0,  0, 10, 10, 10, 10,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr int EARLY_ROOK_SQ[64] = {
    -9,  0,  5, 10, 10, 10,  0, -9,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr int LATE_ROOK_SQ[64] = {
     0,  0, 10, 10, 10, 10,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  2,  2,  2,  2,  0,  0,
     0,  0,  2,  5,  5,  2,  0,  0,
     0,  0,  2,  5,  5,  2,  0,  0,
     0,  0,  2,  2,  2,  2,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
     0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr int EARLY_QUEEN_SQ[64] = {
     0,  0,  0,  5,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, -9, -9, -9, -9,  0,  0,
     0,  0, -9, -9, -9, -9,  0,  0,
     0,  0, -9, -9, -9, -9,  0,  0,
     0,  0, -9, -9, -9, -9,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr int LATE_QUEEN_SQ[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, 50, 50, 50, 50,  0,  0,
     0,  0, 50, 50, 50, 50,  0,  0,
     0,  0, 50, 50, 50, 50,  0,  0,
     0,  0, 50, 50, 50, 50,  0,  0,
    40, 40, 40, 40, 40, 40, 40, 40,
     0,  0,  0,  0,  0,  0,  0,  0,
};

constexpr int EARLY_KING_SQ[64] = {
      0,   0,  10, -20, -20, -20,  80,  70, 
      0,   0,   0, -20, -20, -20, -10,   0, 
      0,   0, -80, -80, -80, -80,   0,   0, 
      0,   0, -80, -80, -80, -80,   0,   0, 
      0,   0, -80, -80, -80, -80,   0,   0, 
      0,   0, -80, -80, -80, -80,   0,   0, 
      0,   0, -80, -80, -80, -80,   0,   0, 
      0,   0,   0,   0,   0,   0,   0,   0, 
};

constexpr int LATE_KING_SQ[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,  20,  20,  20,  20,   0,   0,
      0,  20,  50,  50,  50,  50,  20,   0,
      0,  20,  50,  50,  50,  50,  20,   0,
      0,  20,  50,  50,  50,  50,  20,   0,
      0,  20,  50,  50,  50,  50,  20,   0,
      0,   0,  20,  20,  20,  20,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
};

int eval(const Board&);

} // namespace azalea
