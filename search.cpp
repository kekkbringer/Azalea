#include "search.hpp"

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"
#include "statistics.hpp"

#include <algorithm>
#include <iterator>
#include <chrono>

#include <iostream> // TODO delete
#include "util.hpp" // TODO delete

unsigned long long int nodes, qnodes, tthits;
std::chrono::high_resolution_clock::time_point beginSearch;
extern int movetime;
extern TTentry tTable[ttsize];
bool terminateSearch;

using namespace std::chrono;

/******************************************************************************
* Main iterative deepening search function, calls alphaBeta
*/
void search(GameState& gs, const int depth, const zobristKeys& zobrist) {
    terminateSearch = false;
    movetime *= 0.97;
    movetime -= 5; // take 5ms for flat time buffer
    if constexpr (azalea::statistics)
	outputStats("entering main search with movetime "
						+ std::to_string(movetime));

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
    int alpha = std::numeric_limits<int>::min() + 1;
    int beta  = std::numeric_limits<int>::max() - 1;

    // iterative deepening
    const auto start = high_resolution_clock::now();
    for (int idDepth=1; idDepth<=depth; idDepth++) {
	if constexpr (azalea::statistics)
	    outputStats("\n=== ITERATIVE DEEPENING ITERATION "
				    + std::to_string(idDepth) + " ===\n");

	// check if time is over before starting a new iteration
	// this is needed in mate scenarios where the number of nodes stays
	// extremly small and time termination would not occur in alphaBeta
	if (movetime > 0) {
	    const auto now = high_resolution_clock::now();
	    const auto dur = duration_cast<milliseconds>(now - beginSearch);
	    if (dur.count() >= movetime) {
		if constexpr (azalea::statistics)
		    outputStats("time termination in ID loop\n");
		terminateSearch = true;
		break;
	    }
	}

	if constexpr (azalea::statistics)
	    outputStats("starting search with window:\nalpha: "
	    + std::to_string(alpha) + "\nbeta: " + std::to_string(beta) + "\n");


	tthits = 0;
	// call to core search routine
	pvline.clear();
    	const int score = alphaBeta(gs, alpha, beta, idDepth, 1, zobrist);
	if constexpr (azalea::statistics)
	    outputStats("alphaBeta returned score: "
					    + std::to_string(score) + "\n");


	/**************************************************
	 *             aspiration windows                 *
	 *************************************************/
	if (not terminateSearch) // otherwise 'continue' might result in
				 // an infinite loop
	if (score <= alpha) { // fail low, lower alpha
	    // score is outside of window -> research with full window
	    if constexpr (azalea::statistics)
		outputStats("aspiration window: fail low -> lowering alpha\n");
	    alpha = std::numeric_limits<int>::min();
	    idDepth--;
	    continue;
	}
	if (score >= beta) { // fail high, raise beta
	    // score is outside of window -> research with full window
	    if constexpr (azalea::statistics)
		outputStats("aspiration window: fail high -> raising beta\n");
	    beta = std::numeric_limits<int>::max();
	    idDepth--;
	    continue;
	}

	// set window for next depth
	alpha = score - azalea::aspirationWindowSize;
	beta  = score + azalea::aspirationWindowSize;

	const auto end = high_resolution_clock::now();
	const auto duration = duration_cast<milliseconds>(end-start);

    	// print final search info: depth, score
	std::string output = "info";
    	output += " depth " + std::to_string(idDepth)
		//+ " score cp " + std::to_string(score/10);
		+ " score " + scoreOrMate(score);

	// print number of nodes searched and total time of the iteration
	const int ms = 1 + duration.count(); // millisecs, rounded up
	output += " nodes " + std::to_string(nodes) +
		  " qnodes " + std::to_string(qnodes);
	output += " time " + std::to_string(ms) +
		  " nps " + std::to_string((nodes*1000)/ms);
	output += " tthits " + std::to_string(tthits);

	nodes = 0;
	qnodes = 0;

	// print out how full the transposition table is in permill
	//unsigned int full = 0;
	//for (size_t i=0; i<ttsize; i++) {
	//    if (tTable[i].draft != -1) full++;
	//}
	//output += " hashfull " + std::to_string((int)(full*1000/(ttsize)));

	//// print principle variation
	//std::cout << "pv ";
    	//for (const auto& m: pvline) std::cout << m << " ";

	// debug: print PV from transposition table
	output += " pv ";
	const auto e1 = tTable[gs.zhash%(ttsize)];
	if (e1.zhash == gs.zhash) bestmove = e1.bestmove;
	output += toString(bestmove) + " ";
	auto gscopy = gs;
	gscopy.makeMove(bestmove, zobrist);
	for (int i=0; i<idDepth+5; i++) {
	    const auto entry = tTable[gscopy.zhash%(ttsize)];
	    if (entry.zhash != gscopy.zhash) break;
	    //if (entry.nodeType != NodeType::PVNode) break;
	    output += toString(entry.bestmove) + " ";
	    gscopy.makeMove(entry.bestmove, zobrist);
	}

	output += "\n";
	std::cout << output << std::flush;
	if constexpr (azalea::statistics)
	    outputStats(output);

	// as the hashmove is tried first in every iteration, even the result
	// of a partial search can be used
	if (terminateSearch) break;
    }

    // print final bestmove
    std::cout << "bestmove " << bestmove << std::endl;
    if constexpr (azalea::statistics)
	outputStats("bestmove " + toString(bestmove) + "\n\n\n");
}


/******************************************************************************
 * Fail-soft alpha beta
 */
int alphaBeta(GameState& gs, int alpha, int beta, int depth, int ply,
		const zobristKeys& zobrist) {
    nodes++;
    if (nodes%2048 == 0) {
	if (movetime > 0) {
	    const auto now = high_resolution_clock::now();
	    const auto dur = duration_cast<milliseconds>(now - beginSearch);
	    if (dur.count() >= movetime) {
		if constexpr (azalea::statistics)
		    outputStats("time termination in alphaBeta\n");
		terminateSearch = true;
		return 0;
	    }
	}
	if (nodes%8*1024 == 0) {
	    if (listenForStop()) {
	        terminateSearch = true;
	        return 0;
	    }
	}
    }

    std::vector<Move> movelist;
    bool inCheck;
    generateLegalMoves(gs, movelist, inCheck);

    // check and stale mate detection
    if (movelist.size() == 0) {
        if (inCheck) return azalea::CHECKMATE + ply;
	return 0;
    }

    // singular check extension
    if (inCheck) depth++;

    // check for 3 fold repetition
    // here actually the first repetition will already be resulting in a
    // draw evaluation
    int repetitions = 0;
    for (int i=gs.repPlyCounter-2; i>=0 and i<azalea::repHistMaxPly; i--) {
	if (gs.repHist[i] == gs.zhash) repetitions++;
	// TODO here should really be a comtempt factor which is returned
	if (repetitions == 2) return 0;
	//if (repetitions == 2) return -500;
    }

    // check for transposition table hit
    const auto probeEntry = tTable[gs.zhash%(ttsize)];
    Move hashmove;
    bool tthit = false;
    if (gs.zhash == probeEntry.zhash and probeEntry.draft != -1) { // tthit
        // save transposition table move for move ordering
        hashmove = probeEntry.bestmove;
        tthit = true;

        // if not root and tthit is from a deep enough search, return
        // the score corresponding to the correct node type
        if (ply > 1) {
            if (probeEntry.draft >= depth) { // deep enough
        	if (probeEntry.nodeType == NodeType::PVNode) {
        	    tthits++;
        	    return probeEntry.score;
        	}
        	if (probeEntry.nodeType == NodeType::AlphaNode
        		and probeEntry.score <= alpha) {
        	    tthits++;
        	    return alpha;
        	}
        	if (probeEntry.nodeType == NodeType::BetaNode
        		and probeEntry.score >= beta) {
        	    tthits++;
        	    return beta;
        	}
            }
        }
    }

    int bestscore = azalea::CHECKMATE;
    //int bestscore = azalea::MININT;
    std::vector<Move> line;
    NodeType ttNode = NodeType::AlphaNode;

    // enter quiescence search
    if (depth <= 0) {
	nodes--;
	return qsearch(gs, alpha, beta, zobrist);
    }

    // if we have a tthit, check if ttmove is legal, if so -> sort to front
    if (tthit) {
        if (std::find(movelist.begin(), movelist.end(), hashmove)
        	    != movelist.end()) {
            // for now just insert it to the front, the ttmove is now
            // contained twice in the movelist
	    movelist.erase(std::remove(movelist.begin(), movelist.end(), hashmove),
					movelist.end());
            movelist.insert(movelist.begin(), hashmove);
        }
    }

    Move ttmove;

    // iterate over legal moves
    for (const auto& m: movelist) {
	if (terminateSearch) break;

	auto umi = gs.makeMove(m, zobrist);
	int score = -alphaBeta(gs, -beta, -alpha, depth - 1, ply + 1, zobrist);
	gs.unmakeMove(umi, zobrist);

	if (score > bestscore) {
	    bestscore = score;
	    ttmove = m;
	}

	// beta cutoff
	if (score >= beta) {
	    ttNode = NodeType::BetaNode;
	    break;
	}

	// raising alpha?
	if (score > alpha) {
	    ttNode = NodeType::PVNode;
	    alpha = score;
	}
    }

    // store search info in transposition table
    if (not terminateSearch) {
	TTentry entry;
	entry.zhash = gs.zhash;
	entry.bestmove = ttmove;
	entry.draft = depth;
	entry.score = bestscore;
	entry.nodeType = ttNode;

	const auto curEntry = tTable[gs.zhash%ttsize];
	if (curEntry.nodeType != NodeType::PVNode)
	    //and entry.draft >= curEntry.draft)
		    tTable[gs.zhash%ttsize] = entry;
    }

    return bestscore;
}
