#pragma once

#include "board.hpp"
#include "move.hpp"

#include <vector>


/******************************************************************************
 * Main search function, calls alphaBeta
 */
void search(const GameState& gs, const int depth);

/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(const GameState& gs, int alpha, int beta, int depth, int ply,
		    std::vector<Move>& pvline, const std::vector<Move>& oldpv);

/******************************************************************************
 * Quiescence search
 */
int qsearch(const GameState& gs, int alpha, int beta);