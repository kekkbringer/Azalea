#pragma once

#include "board.hpp"
#include "move.hpp"

/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(const GameState& gs, Move& bestmove,
		    int alpha, int beta, int depth, int ply);

/******************************************************************************
 * Quiescence search
 */
int qsearch(const GameState& gs, int alpha, int beta);
