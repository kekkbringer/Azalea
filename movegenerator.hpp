#pragma once

#include <vector>

#include "params.hpp"
#include "board.hpp"
#include "move.hpp"

/******************************************************************************
 * Function that generates all legal moves in a given position. Returns the
 * number of legal moves and gives the moves themself on 'movelist'.
 */
void generateLegalMoves(const Board& b, std::vector<Move>& movelist);

bitb rookAttacks(const bitb occ, const int index);
bitb bishopAttacks(const bitb occ, const int index);
