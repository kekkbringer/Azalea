#include "search.hpp"

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"

#include <algorithm>
#include <iterator>
#include <chrono>

#include <iostream> // TODO delete
#include "util.hpp" // TODO delete


unsigned long long int nodes, qnodes;
std::chrono::high_resolution_clock::time_point beginSearch;
extern int movetime;
bool terminateSearch;

using namespace std::chrono;

/******************************************************************************
* Main iterative deepening search function, calls alphaBeta
*/
void search(GameState& gs, const int depth, const zobristKeys& zobrist) {
    terminateSearch = false;
    movetime *= 0.97; // only use 97% of time allocated to be safe

    beginSearch = high_resolution_clock::now();

    std::vector<Move> pvline, oldPV;
    Move bestmove;

    // sometimes the first iteration takes longer than the movetime given, for
    // this case just return the first legal move generated
    {
	std::vector<Move> movelist;
    	bool inCheck;
    	generateLegalMoves(gs, movelist, inCheck);
	bestmove = movelist[0];
    }

    // set alpha and beta to inf for the first iteration
    int alpha = azalea::MININT;
    int beta  = azalea::MAXINT;

    // "dumb" iterative deepening
    const auto start = high_resolution_clock::now();
    for (int idDepth=1; idDepth<=depth; idDepth++) {

	nodes = 0;
	qnodes = 0;
	// call to core search routine
	pvline.clear();
    	const int score = alphaBeta(gs, alpha, beta, idDepth, 0, pvline,
				    bestmove, zobrist);
	if (terminateSearch) break;
	bestmove = pvline[0]; // this is only needed as the search currently
			      // does not try the bestmove of the previous
			      // iteration first...
	oldPV = pvline;

	/**************************************************
	 *             aspiration windows                 *
	 *************************************************/
	if (score <= alpha) { // fail low, lower alpha
	    // score is outside of window -> research with full window
	    alpha = azalea::MININT;
	    idDepth--;
	    continue;
	}
	if (score >= beta) { // fail high, raise beta
	    // score is outside of window -> research with full window
	    beta  = azalea::MAXINT;
	    idDepth--;
	    continue;
	}
	// set window for next depth
	alpha = score - azalea::aspirationWindowSize;
	beta  = score + azalea::aspirationWindowSize;

	const auto end = high_resolution_clock::now();
	const auto duration = duration_cast<milliseconds>(end-start);

    	// print final search info: depth, score
    	std::cout << "info depth " << idDepth << " score cp " << score/10;

	// print number of nodes searched and total time of the iteration
	const int ms = 1 + duration.count(); // millisecs, rounded up
	std::cout << " nodes " << nodes << " qnodes " << qnodes;
	std::cout<< " time " << ms << " nps " << (nodes*1000)/ms;

	// print principle variation
	std::cout << " pv ";
    	for (const auto& m: pvline) std::cout << m << " ";
    	std::cout << std::endl;
    }

    // print final bestmove
    std::cout << "bestmove " << bestmove << std::endl;
}


/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(GameState& gs, int alpha, int beta, int depth, int ply,
		std::vector<Move>& pvline, Move& bestmove,
	    
		const zobristKeys& zobrist) {
    nodes++;
    if (nodes%2048 == 0) {
	if (movetime > 0) {
	    const auto now = high_resolution_clock::now();
	    const auto dur = duration_cast<milliseconds>(now - beginSearch);
	    if (dur.count() >= movetime) {
		terminateSearch = true;
		return alpha;
	    }
	}
	if (nodes%8*1024 == 0) {
	    if (listenForStop()) {
	        terminateSearch = true;
	        return alpha;
	    }
	}
    }

    int bestscore = azalea::MININT;
    std::vector<Move> line;

    std::vector<Move> movelist;
    bool inCheck;
    generateLegalMoves(gs, movelist, inCheck);

    // singular check extension
    if (inCheck) depth++;

    if (movelist.size() == 0) {
        if (inCheck) return azalea::MININT + 100 + ply;
	return 0;
    }

    // enter quiescence search
    if (depth <= 0) {
	pvline.resize(0);
	return qsearch(gs, alpha, beta, zobrist);
    }

    if (ply == 0 and depth > 1 and !inCheck) {
        movelist.erase(std::remove(movelist.begin(), movelist.end(), bestmove),
		       movelist.end());
        movelist.insert(movelist.begin(), bestmove);
    }

    // iterate over legal moves
    for (const auto& m: movelist) {
	if (terminateSearch) break;

	auto umi = gs.makeMove(m, zobrist);
	int score = -alphaBeta(gs, -beta, -alpha, depth - 1, ply + 1,
			       line, bestmove, zobrist);
	gs.unmakeMove(umi, zobrist);

	if (score > bestscore) {
	    bestscore = score;
	}

	// beta cutoff
	if (score >= beta) {
	    break;
	}

	// raising alpha?
	if (score > alpha) {
	    alpha = score;
	    pvline.clear();
	    pvline.push_back(m);
	    copy(line.begin(), line.end(), back_inserter(pvline));
	}

	// clear line for next search iteration
	line.clear();
    }

    return bestscore;
}
