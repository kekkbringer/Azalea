#include "movegenerator.hpp"

#include <vector>

#include "params.hpp"
#include "board.hpp"
#include "move.hpp"
#include "raylookup.hpp"

/******************************************************************************
 * Function that generates all legal moves in a given position. Returns the
 * number of legal moves and gives the moves themself on 'movelist'.
 */
void generateLegalMoves(const Board& b, std::vector<Move>& movelist) {
    movelist.resize(0);
}

bitb rookAttacks(const bitb occ, const int index) {
    bitb atks = 0ULL;
    bitb atkray, blocker;
    int sq; 

    // north
    atkray = nRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = __builtin_ffsll(blocker) - 1;
	atkray ^= nRays[sq];
    }
    atks |= atkray;

    // east
    atkray = eRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = __builtin_ffsll(blocker) - 1;
	atkray ^= eRays[sq];
    }
    atks |= atkray;

    // south
    atkray = sRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = 63 - __builtin_clzll(blocker);
	atkray ^= sRays[sq];
    }
    atks |= atkray;

    // west
    atkray = wRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = 63 - __builtin_clzll(blocker);
	atkray ^= wRays[sq];
    }
    atks |= atkray;

    return atks;
}

bitb bishopAttacks(const bitb occ, const int index) {
    bitb atks = 0ULL;
    bitb atkray, blocker;
    int sq; 

    // north east
    atkray = neRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = __builtin_ffsll(blocker) - 1;
	atkray ^= neRays[sq];
    }
    atks |= atkray;

    // north west
    atkray = nwRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = __builtin_ffsll(blocker) - 1;
	atkray ^= nwRays[sq];
    }
    atks |= atkray;

    // south west
    atkray = swRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = 63 - __builtin_clzll(blocker);
	atkray ^= swRays[sq];
    }
    atks |= atkray;

    // south east
    atkray = seRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = 63 - __builtin_clzll(blocker);
	atkray ^= seRays[sq];
    }
    atks |= atkray;

    return atks;
}
