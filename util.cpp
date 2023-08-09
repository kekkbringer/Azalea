#include "util.hpp"

#include <string>

#include "board.hpp"

/******************************************************************************
 * Function that converts a square in algebraic notation, e.g. 'e4' to its
 * corresponding index on the bitboard representation,
 */
int toIndex(std::string s) {
    int index = 0;
    index += (s[1]-'1')*8;
    index += 7-(s[0]-'a');
    return index;
}
