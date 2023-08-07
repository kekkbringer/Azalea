#pragma once

/******************************************************************************
 * This file includes some utility functions...
 *****************************************************************************/


#include <string>

/******************************************************************************
 * Function that converts a square in algebraic notation, e.g. 'e4', to its
 * corresponding index on the 10x12 board.
 */
size_t toIndex(std::string s);
