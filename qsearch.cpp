#include "pvssearch.hpp"

#include <chrono>

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"
#include "util.hpp"
#include "statistics.hpp"

using namespace std::chrono;

extern int movetime;
extern bool terminateSearch;
extern unsigned long long int qnodes;

constexpr int INF = std::numeric_limits<int>::max()-1;

std::vector<int> scoreMoves(const GameState& gs, const std::vector<Move>& ml, const Move& hashmove);
void sortMove(std::vector<Move>& ml, std::vector<int>& scores, const int& n);

/******************************************************************************
 * Quiescence search
 */
int qsearch(GameState& gs, int alpha, int beta, const zobristKeys& zobrist) {
    qnodes++;
    int standpat = eval<false>(gs);

    if (qnodes%2048 == 0) {
	if (movetime > 0) {
	    if (measureTimeSinceStart() >= movetime) {
		if constexpr (azalea::statistics)
		    outputStats("time termination in qsearch loop\n");
		terminateSearch = true;
		return 0;
	    }
	}
	if (qnodes%(8*1024) == 0) {
	    if (listenForStop()) {
	        terminateSearch = true;
	        return 0;
	    }
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

    // move ordering
    const Move dummove;
    auto scores = scoreMoves(gs, movelist, dummove);
    
    for (size_t n=0; n<movelist.size(); n++) {
	sortMove(movelist, scores, n);
	const auto& m = movelist[n];
	if (!m.capture and !(m.promo and m.promoPiece=='q')) continue;
	auto umi = gs.makeMove(m, zobrist);

	/* delta pruning
    	int delta = 0;
	switch (umi.capturedPiece) {
	    case pieceType::pawn: delta = 1000; break;
	    case pieceType::knight: delta = 3000; break;
	    case pieceType::bishop: delta = 3250; break;
	    case pieceType::rook: delta = 5000; break;
	    case pieceType::queen: delta = 9000; break;
	}
	if (standpat+delta+azalea::deltaMargin < alpha and !umi.promotion) {
	    gs.unmakeMove(umi, zobrist);
	    continue;
	}
    	// end of delta pruning */

	int score = -qsearch(gs, -beta, -alpha, zobrist);
	gs.unmakeMove(umi, zobrist);

	if (terminateSearch) break;

	if (score >= beta) {
	    return beta;
	}
	if (score > alpha) {
	    alpha = score;
	}
    }

    return alpha;
}
