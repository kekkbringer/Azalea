#include "search.hpp"

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"

#include <iostream> // TODO delete
#include "util.hpp" // TODO delete

/******************************************************************************
 * Quiescence search
 */
int qsearch(const GameState& gs, int alpha, int beta) {
    int standpat = eval(gs);

    if (standpat >= beta) {
	return beta;
    }
    if (alpha < standpat) {
	alpha = standpat;
    }

    std::vector<Move> movelist;
    bool inCheck;
    generateLegalMoves(gs, movelist, inCheck);
    
    for (const auto& m: movelist) {
	if (!m.capture) continue;

	auto dummy = gs;
	dummy.makeMove(m);

	int score = -qsearch(dummy, -beta, -alpha);

	if (score >= beta) {
	    return beta;
	}
	if (score > alpha) {
	    alpha = score;
	}
    }

    return alpha;
}
