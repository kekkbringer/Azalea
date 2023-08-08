#pragma once

/******************************************************************************
 * This file includes some utility functions...
 *****************************************************************************/


#include <string>

#include "board.hpp"

/******************************************************************************
 * Function that converts a square in algebraic notation, e.g. 'e4', to its
 * corresponding index on the 10x12 board.
 */
size_t toIndex(std::string s);

/******************************************************************************
 * Function that checks if a give square is attacked by the enemy. Own King is
 * not a blocker, this is for movegen purposes.
 */
bool isAttackedNoKing(const Board& b, const int index);
