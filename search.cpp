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


int nodes;


/******************************************************************************
 * Main iterative deepening search function, calls alphaBeta
 */
void search(const GameState& gs, const int depth) {
    using namespace std::chrono;

    Move bestmove;
    std::vector<Move> pvline;

    // "dumb" iterative deepening
    for (int idDepth=1; idDepth<=depth; idDepth++) {
	const auto start = high_resolution_clock::now();

	nodes = 0;
	// call to core search routine
	pvline.clear();
    	const int score = alphaBeta(gs, bestmove, azalea::MININT, azalea::MAXINT,
    	    						idDepth, 0, pvline);

	const auto end = high_resolution_clock::now();
	const auto duration = duration_cast<milliseconds>(end-start);

    	// print final search info: depth, score
    	std::cout << "info depth " << idDepth << " score cp " << score/10;

	// print number of nodes searched and total time of the iteration
	std::cout << " nodes " << nodes << " time " << duration.count();

	// print principle variation
	std::cout << " pv ";
    	for (const auto& m: pvline) std::cout << m << " ";
    	std::cout << std::endl;
    }

    // print final bestmove
    std::cout << "bestmove " << bestmove << std::endl;
}

/******************************************************************************
 * Benchmark function, calls alphaBeta outside of an ID framework
 */
void searchNOID(const GameState& gs, const int depth) {
    Move bestmove;
    std::vector<Move> pvline;

    // call to core search routine
    const int score = alphaBeta(gs, bestmove, azalea::MININT, azalea::MAXINT,
        						depth, 0, pvline);

    // print final search info
    std::cout << "info depth " << depth << " score cp " << score/10 << " pv ";
    for (const auto& m: pvline) std::cout << m << " ";
    std::cout << std::endl;
    std::cout << "bestmove " << bestmove << std::endl;
}  


/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(const GameState& gs, Move& bestmove,
			    int alpha, int beta, int depth, int ply,
			    std::vector<Move>& pvline) {
    nodes++;

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

    // TODO quiescence search
    if (depth <= 0) {
	pvline.resize(0);
	return qsearch(gs, alpha, beta);
    }

    // iterate over legal moves
    for (const auto& m: movelist) {
	auto dummy = gs;
	dummy.makeMove(m);

	int score = -alphaBeta(dummy, bestmove, -beta, -alpha,
				depth - 1, ply + 1, line);

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
	    pvline.clear();
	    pvline.push_back(m);
	    copy(line.begin(), line.end(), back_inserter(pvline));
	}

	// clear line for next search iteration
	line.clear();
    }

    return bestscore;
}
