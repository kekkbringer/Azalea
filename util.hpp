#pragma once

#include <string>

/******************************************************************************
 * This file includes some utility functions...
 *****************************************************************************/


/******************************************************************************
 * Function that converts a square in algebraic notation, e.g. 'e4' to its
 * corresponding index on the bitboard representation,
 */
int toIndex(std::string s);
