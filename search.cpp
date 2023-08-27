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


unsigned long long int nodes;


/******************************************************************************
 * Main iterative deepening search function, calls alphaBeta
 */
void search(const GameState& gs, const int depth) {
    using namespace std::chrono;

    std::vector<Move> pvline, oldpv;

    int alpha = azalea::MININT;
    int beta  = azalea::MAXINT;

    // "dumb" iterative deepening
    for (int idDepth=1; idDepth<=depth; idDepth++) {
	const auto start = high_resolution_clock::now();

	nodes = 0;
	// call to core search routine
	pvline.clear();
    	const int score = alphaBeta(gs, alpha, beta,
    	    			    idDepth, 0, pvline, oldpv);
	oldpv = pvline;

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
	std::cout << " nodes " << nodes << " time " << ms;
	std::cout << " nps " << (nodes*1000)/ms;

	// print principle variation
	std::cout << " pv ";
    	for (const auto& m: pvline) std::cout << m << " ";
    	std::cout << std::endl;
    }

    // print final bestmove
    std::cout << "bestmove " << pvline[0] << std::endl;
}

/******************************************************************************
 * Benchmark function, calls alphaBeta outside of an ID framework
 */
void searchNOID(const GameState& gs, const int depth) {
    std::vector<Move> pvline, oldpv;

    // call to core search routine
    const int score = alphaBeta(gs, azalea::MININT, azalea::MAXINT,
        			depth, 0, pvline, oldpv);

    // print final search info
    std::cout << "info depth " << depth << " score cp " << score/10 << " pv ";
    for (const auto& m: pvline) std::cout << m << " ";
    std::cout << std::endl;
    std::cout << "bestmove " << pvline[0] << std::endl;
}  


/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(const GameState& gs, int alpha, int beta, int depth, int ply,
		std::vector<Move>& pvline, const std::vector<Move>& oldpv) {
    nodes++;
    //if (nodes%2048 == 0) {
    //    if (listenForStop()) return alpha;
    //}

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
		return qsearch(gs, alpha, beta);
    }

    //// firstly, try oldpv move if available
    //if (ply==0) {
    //    auto dummy = gs;
    //    for (auto m: oldpv) dummy.makeMove(m);
    //    // TODO: second '1' in next line is 'wrong'
    //    int score = -alphaBeta(dummy, -beta, -alpha, 1, 1,
    //    							line, oldpv);
    //    bestscore = score;
    //    if (score > beta) return bestscore;
    //    if (score > alpha) {
    //        alpha = score;
    //        pvline.clear();
    //        for (auto m: oldpv) pvline.push_back(m);
    //        //pvline.push_back(line[0]);
    //    }
    //    line.clear();
    //}

    // iterate over legal moves
    for (const auto& m: movelist) {
	auto dummy = gs;
	dummy.makeMove(m);

	int score = -alphaBeta(dummy, -beta, -alpha,
				depth - 1, ply + 1, line, oldpv);

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
