#pragma once

#include "board.hpp"
#include "move.hpp"
#include "params.hpp"

#include <chrono>

using namespace std::chrono;

extern high_resolution_clock::time_point start;

int searchRoot(GameState& gs, int alpha, int beta,
	       int curDepth, const zobristKeys& zobrist, Move& bestmove);

inline auto measureTimeSinceStart() {
    const auto now = high_resolution_clock::now();
    const auto dur = duration_cast<milliseconds>(now - start);
    return dur.count();
}

/******************************************************************************
 * This is the main search driver, realized by a iterative deepening framework.
 *
 * TODO add comment D:
 *
 */
void search(GameState& gs, const int depth, const zobristKeys& zobrist);

