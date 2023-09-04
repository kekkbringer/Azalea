#include "search.hpp"

#include <chrono>

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"

using namespace std::chrono;

extern int movetime;
extern high_resolution_clock::time_point beginSearch;
extern bool terminateSearch;
extern unsigned long long int qnodes;


/******************************************************************************
 * Quiescence search
 */
int qsearch(GameState& gs, int alpha, int beta) {
    qnodes++;
    int standpat = eval(gs);

    if (qnodes%2048 == 0) {
	const auto now = high_resolution_clock::now();
	const auto dur = duration_cast<milliseconds>(now - beginSearch);
	if (dur.count() >= movetime and movetime > 0) {
	    terminateSearch = true;
	    return standpat;
	}
    }

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
	if (!m.capture and !(m.promo and m.promoPiece=='q')) continue;
	if (terminateSearch) break;

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
