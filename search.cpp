#include "search.hpp"

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"

#include <iostream> // TODO delete
#include "util.hpp" // TODO delete

/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(const GameState& gs, Move& bestmove,
			    int alpha, int beta, int depth, int ply) {
    int bestscore = azalea::MININT;

    std::vector<Move> movelist;
    bool inCheck;
    generateLegalMoves(gs, movelist, inCheck);

    // singular check extension
    if (inCheck) depth++;

    if (movelist.size() == 0) {
        if (inCheck) return azalea::MININT + 100 + ply;
	return 0;
    }

    // TODO quiescence search
    //if (depth <= 0) return eval(gs);
    if (depth <= 0) return qsearch(gs, alpha, beta);

    // iterate over legal moves
    for (const auto& m: movelist) {
	auto dummy = gs;
	dummy.makeMove(m);

	int score =
	    -alphaBeta(dummy, bestmove, -beta, -alpha, depth - 1, ply + 1);

	if (score > bestscore) {
	    bestscore = score;
	    if (ply == 0) bestmove = m;
	}

	// beta cutoff
	if (score >= beta) {
	    break;
	}

	// raising alpha?
	if (score > alpha) {
	    alpha = score;
	}
    }

    return bestscore;
}
