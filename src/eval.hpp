#pragma once

#include "board.hpp"
#include "engine.hpp"

#include <cstdlib> // std::abs

namespace azalea {

constexpr int SCORE_INF  = 1048576;
constexpr int SCORE_MATE = 1048575;
constexpr int MATE_BOUND = SCORE_MATE - MAX_PLY;

constexpr bool is_mate_score(int s) { return std::abs(s) >= MATE_BOUND; }

constexpr int PAWN_EVAL   = 100;
constexpr int KNIGHT_EVAL = 300;
constexpr int BISHOP_EVAL = 320;
constexpr int ROOK_EVAL   = 500;
constexpr int QUEEN_EVAL  = 900;

int eval(const Board&);

} // namespace azalea
