#pragma once

#include "board.hpp"
#include "move.hpp"
#include "zobrist.hpp"

#include <vector>


/******************************************************************************
 * Main search function, calls alphaBeta
 */
void search(GameState& gs, const int depth, const zobristKeys& zobrist);

/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(GameState& gs, int alpha, int beta, int depth, int ply,
		    const zobristKeys& zobrist);

/******************************************************************************
 * Quiescence search
 */
int qsearch(GameState& gs, int alpha, int beta, const zobristKeys& zobrist);
