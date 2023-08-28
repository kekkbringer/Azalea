#pragma once

#include "board.hpp"
#include "move.hpp"

#include <vector>


/******************************************************************************
 * Main search function, calls alphaBeta
 */
void search(GameState& gs, const int depth);
void searchNOID(GameState& gs, const int depth);

/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(GameState& gs, int alpha, int beta, int depth, int ply,
		    std::vector<Move>& pvline, const std::vector<Move>& oldpv);

/******************************************************************************
 * Quiescence search
 */
int qsearch(GameState& gs, int alpha, int beta);
