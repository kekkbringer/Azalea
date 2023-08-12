#include "search.hpp"

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"

/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(const GameState& gs, Move& bestmove,
			    int alpha, int beta, int depth, int ply) {
    int bestscore = azalea::MININT;

    std::vector<Move> movelist;
    bool inCheck;
    generateLegalMoves(gs, movelist, inCheck);

    // TODO quiescence search
    if (depth == 0) return eval(gs, movelist.size(), inCheck);

    // iterate over legal moves
    for (const auto& m: movelist) {
	auto dummy = gs;
	dummy.makeMove(m);

	int score =
	    -alphaBeta(dummy, bestmove, -beta, -alpha, depth - 1, ply + 1);

	if (score >= alpha) {
	    alpha = score;
	    bestmove = m;
	}

	if (alpha >= beta) break;

    }

    return alpha;
}
