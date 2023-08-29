#include "search.hpp"

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"


/******************************************************************************
 * Quiescence search
 */
int qsearch(GameState& gs, int alpha, int beta) {
    int standpat = eval(gs);

    if (standpat >= beta) {
	return beta;
    }
    if (alpha < standpat) {
	alpha = standpat;
    }

    std::vector<Move> movelist;
    bool inCheck;
    const int nmoves = generateLegalMoves(gs, movelist, inCheck);
    
    for (int i=0; i<nmoves; i++) {
	const auto& m = movelist[i];

	if (!m.capture) continue;

	auto umi = gs.makeMove(m);
	int score = -qsearch(gs, -beta, -alpha);
	gs.unmakeMove(umi);

	if (score >= beta) {
	    return beta;
	}
	if (score > alpha) {
	    alpha = score;
	}
    }

    return alpha;
}
