#pragma once

#include <string>

#include "move.hpp"

/******************************************************************************
 * This file includes some utility functions...
 *****************************************************************************/


/******************************************************************************
 * Function that converts a square in algebraic notation, e.g. 'e4' to its
 * corresponding index on the bitboard representation,
 */
int toIndex(std::string s);

/******************************************************************************
 * Function that converts a 'Move' to a string corresponding to algeraic
 * notation, e.g. "e2e4" or "b7b8q".
 */
std::string toString(const Move& m);

/******************************************************************************
 * Overload of function for printing 'Move's to std::ostreams.
 */
std::ostream& operator<<(std::ostream& os, const Move& m);

/******************************************************************************
 * Function that converts a index on the bitboard to its corresponding square
 * in algebraic notation, e.g. "e4"
 */
std::string_view toString(int index);
