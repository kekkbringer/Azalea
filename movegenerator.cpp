#include "movegenerator.hpp"

#include <vector>

#include "params.hpp"
#include "statistics.hpp"
#include "board.hpp"
#include "move.hpp"
#include "util.hpp"

/******************************************************************************
 * Function that generates all legal moves in a given position. Returns the
 * number of legal moves and gives the moves themself on 'movelist'.
 */
void generateLegalMoves(const Board& b, std::vector<Move>& movelist) {
    movelist.resize(0);
}
