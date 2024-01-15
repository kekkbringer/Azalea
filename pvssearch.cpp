#include "pvssearch.hpp"

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "params.hpp"
#include "eval.hpp"
#include "statistics.hpp"
#include "util.hpp"

#include <algorithm>
#include <iterator>
#include <chrono>

using namespace std::chrono;

unsigned long long int nodes, qnodes, tthits;
bool terminateSearch;

extern int movetime;
extern TTentry tTable[ttsize];

constexpr int INF = std::numeric_limits<int>::max()-1;

int qsearch(GameState& gs, int alpha, int beta, const zobristKeys& zobrist);
int alphaBeta(GameState& gs, int beta, int alpha, int curDepth,
	      int ply, const zobristKeys& zobrist);

// checks for three-fold repetition
bool checkThreeFoldRep(const GameState& gs) {
    int repetitions = 0;
    for (int i=gs.repPlyCounter-2; i>=0 and i<azalea::repHistMaxPly; i-=2) {
        if (gs.repHist[i] == gs.zhash) repetitions++;
	if (repetitions == 2) {
	    return true;
	}
    }
    return false;
}

// scores the moves given in the movelist
constexpr int HASHSCORE = std::numeric_limits<int>::max();
std::vector<int> scoreMoves(const GameState& gs, const std::vector<Move>& ml, const Move& hashmove) {
    const auto& b = gs.board;
    std::vector<int> scores;
    for (const auto& m: ml) {
	if (m == hashmove) {
	    scores.push_back(HASHSCORE);
	    continue;
	}
	const auto fromMask = (1ULL << m.from);
	const auto toMask = (1ULL << m.to);
	if (m.capture and m.promo) {
	    int score = 100000 - 100;
	    if (m.promoPiece == 'q') { score += 900;
	    } else if (m.promoPiece == 'r') { score += 500;
	    } else if (m.promoPiece == 'b') { score += 325;
	    } else if (m.promoPiece == 'n') { score += 300;
	    }
	    if (toMask & (b.wPawns | b.bPawns)) { score += 100;
	    } else if (toMask & (b.wKnights | b.bKnights)) { score += 300;
	    } else if (toMask & (b.wBishops | b.bBishops)) { score += 325;
	    } else if (toMask & (b.wRooks | b.bRooks)) { score += 500;
	    } else if (toMask & (b.wQueens | b.bQueens)) { score += 900;
	    }
	    scores.push_back(score);
	} else if (m.capture) {
	    int score = 100000;
	    if (fromMask & (b.wPawns | b.bPawns)) { score -= 100;
	    } else if (fromMask & (b.wKnights | b.bKnights)) { score -= 300;
	    } else if (fromMask & (b.wBishops | b.bBishops)) { score -= 325;
	    } else if (fromMask & (b.wRooks | b.bRooks)) { score -= 500;
	    } else if (fromMask & (b.wQueens | b.bQueens)) { score -= 900;
	    } else if (fromMask & (b.wKing | b.bKing)) { score -= 1000;
	    }
	    if (toMask & (b.wPawns | b.bPawns)) { score += 100;
	    } else if (toMask & (b.wKnights | b.bKnights)) { score += 300;
	    } else if (toMask & (b.wBishops | b.bBishops)) { score += 325;
	    } else if (toMask & (b.wRooks | b.bRooks)) { score += 500;
	    } else if (toMask & (b.wQueens | b.bQueens)) { score += 900;
	    }
	    scores.push_back(score);
	} else if (m.promo) {
	    int score = 100000;
	    if (m.promoPiece == 'q') { score += 900;
	    } else if (m.promoPiece == 'r') { score += 500;
	    } else if (m.promoPiece == 'b') { score += 325;
	    } else if (m.promoPiece == 'n') { score += 300;
	    }
	    scores.push_back(score);
	} else {
	    scores.push_back(0);
	}
    }
    return scores;
}

// insertion sort that sort up to a certain move
void sortMove(std::vector<Move>& ml, std::vector<int>& scores, const int& n) {
    // the first n moves are already
    // take first unsorted move a initial bestscore
    int bestscore = scores[n];
    int bestindex = n;

    // look for best move in unordered part
    for (size_t i=n; i<ml.size(); i++) {
	if (scores[i] > bestscore) {
	    bestindex = i;
	    bestscore = scores[i];
	}
    }

    // swap moves
    const auto m = ml[bestindex];
    ml[n] = ml[bestindex];
    ml[bestindex] = m;
    // swap scores
    scores[bestindex] = scores[n];
    scores[n] = bestscore;
}

/******************************************************************************
 * This is the main search driver, realized by a iterative deepening framework.
 *
 * TODO add comment D:
 *
 */
void search(GameState& gs, const int depth, const zobristKeys& zobrist) {
    ///////////////////////////////////////////////////////////////
    //                       INITIALIZATION                      //
    ///////////////////////////////////////////////////////////////
    start = high_resolution_clock::now(); // start clock

    // output for profiling
    if constexpr (azalea::statistics)
	outputStats("entering main search with movetime "
		    + std::to_string(movetime));

    nodes = 0;
    qnodes = 0;
    Move bestmove;
    terminateSearch = false;
    movetime = std::max((int)0.97*movetime, movetime-5); // add a small time buffer
    
    // search with a full window for the first iteration
    int alpha = -INF;
    int beta  = INF;

    ///////////////////////////////////////////////////////////////
    //                 ITERATIVE DEEPENING LOOP                  //
    ///////////////////////////////////////////////////////////////
    for (int curDepth=1; curDepth<=depth; curDepth++) {
	if constexpr (azalea::statistics)
	    outputStats("\n=== ITERATIVE DEEPENING ITERATION "
			+ std::to_string(curDepth) + " ===\n");

	// check if allocated search time if over before starting a new
	// iteration
	if (movetime > 0) {
	    if (measureTimeSinceStart() >= movetime) {
		if constexpr (azalea::statistics)
		    outputStats("time termination in ID loop\n");
		terminateSearch = true;
		break;
	    }
	}

	// start actual search
	if constexpr (azalea::statistics)
	    outputStats("starting search with window:\nalpha: "
	    + std::to_string(alpha) + "\nbeta: " + std::to_string(beta) + "\n");

	const int score = searchRoot(gs, alpha, beta, curDepth,
				     zobrist, bestmove);
	if constexpr (azalea::statistics)
	    outputStats("searchRoot returned score: "
			+ std::to_string(score) + "\n");

	if (terminateSearch) break;

           /////////////////////////////////////////////////
           //             ASPIRATION WINDOWS              //
           /////////////////////////////////////////////////
	if (score <= alpha) { // fail low --> lower alpha
	    // score is outside of window -> research with full window
	    if constexpr (azalea::statistics)
		outputStats("aspiration window: fail low -> lowering alpha\n");
	    alpha = -INF;
	    curDepth--;
	    continue;
	}
	if (score >= beta) { // fail high --> raise beta
	    // score is outside of window -> research with full window
	    if constexpr (azalea::statistics)
		outputStats("aspiration window: fail high -> raising beta\n");
	    beta = INF;
	    curDepth--;
	    continue;
	}
	// set window for next iteration
	alpha = score - azalea::aspirationWindowSize;
	beta  = score + azalea::aspirationWindowSize;
	
	// prepare output
	std::string output = "info";
	const auto searchTime = measureTimeSinceStart();

	output += " depth " + std::to_string(curDepth)
		+ " score " + scoreOrMate(score)
		+ " nodes " + std::to_string(nodes)
		+ " qnodes " + std::to_string(qnodes)
		+ " time " + std::to_string(searchTime)
		+ " nps " + std::to_string((nodes*1000)/std::max(searchTime, 1L));

	// retrieve principle variation TODO
	output += " pv " + toString(bestmove) + " ";
	auto gscopy = gs;
	gscopy.makeMove(bestmove, zobrist);
	for (int i=0; i<curDepth+5; i++) {
	    const auto entry = tTable[gscopy.zhash%ttsize];
	    if (entry.zhash != gscopy.zhash) break;
	    if (entry.nodeType != NodeType::PVNode) break;
	    output += toString(entry.bestmove) + " ";
	    gscopy.makeMove(entry.bestmove, zobrist);
	}

	output += "\n";
	std::cout << output << std::flush;
	if constexpr (azalea::statistics)
	    outputStats(output);

    } // end of iterative deepening loop

    // output final bestmove
    std::cout << "bestmove " << bestmove << std::endl;
    if constexpr (azalea::statistics)
	outputStats("bestmove " + toString(bestmove) + "\n\n\n");
}

// just a fail-soft alpha beta for now
int searchRoot(GameState& gs, int alpha, int beta,
	int curDepth, const zobristKeys& zobrist, Move& bestmove) {
    // check for termination conditions
    if (nodes%2048 == 0) {
	if (movetime > 0) {
	    if (measureTimeSinceStart() >= movetime) {
		if constexpr (azalea::statistics)
		    outputStats("time termination in searchRoot loop\n");
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
        if (inCheck) return azalea::CHECKMATE;
	return 0;
    }

    // singular check extension
    if (inCheck) curDepth++;

    // check for 3 fold repetition
    if (checkThreeFoldRep(gs)) return 0;

    // if we have a bestmove from a previous iteration use that first
    // that way we can use the results of a partial search in case of
    // an early termination during the search
    if (not (bestmove.from == 0 and bestmove.to == 0)) {
	if (std::find(movelist.begin(), movelist.end(), bestmove)
		!= movelist.end()) {
	    movelist.erase(std::remove(movelist.begin(), movelist.end(),
			    bestmove), movelist.end());
	    movelist.insert(movelist.begin(), bestmove);
	}
    }

    nodes++;
    int bestscore = -INF;
    NodeType ttNode = NodeType::AlphaNode;


    // move ordering
    auto scores = scoreMoves(gs, movelist, bestmove);
    sortMove(movelist, scores, 0);

    // ================== PVS first move ==================
    const auto umi = gs.makeMove(movelist[0], zobrist);
    int score = -alphaBeta(gs, -beta, -alpha, curDepth-1, 1, zobrist);
    gs.unmakeMove(umi, zobrist);

    bestscore = score;
    bestmove = movelist[0];

    // beta cutoff
    if (score >= beta) {
	ttNode = NodeType::BetaNode;
	goto saveTTAndReturn;
    }

    // raising alpha
    if (score > alpha) {
	ttNode = NodeType::PVNode;
	alpha = score;
    }
    // ================== end of PVS first move ==================


    // iterate over legal moves
    for (size_t n=1; n<movelist.size(); n++) {
	sortMove(movelist, scores, n);
	const auto& m = movelist[n];

	const auto umi = gs.makeMove(m, zobrist);
	int score = -alphaBeta(gs, -alpha-1, -alpha, curDepth-1, 1, zobrist);
	if (score > alpha and score < beta) {
	    // research needed, use full window
	    score = -alphaBeta(gs, -beta, -alpha, curDepth-1, 1, zobrist);
	    if (score > alpha) {
		ttNode = NodeType::PVNode;
		alpha = score;
	    }
	}
	gs.unmakeMove(umi, zobrist);

	if (terminateSearch) break;

	if (score > bestscore) {
	    bestscore = score;
	    bestmove = m;
	}

	// beta cutoff
	if (score >= beta) {
	    ttNode = NodeType::BetaNode;
	    break;
	}

	// raising alpha
	if (score > alpha) {
	    ttNode = NodeType::PVNode;
	    alpha = score;
	}
    }

    // store search info in transposition table
saveTTAndReturn:
    if (not terminateSearch) {
	TTentry entry;
	entry.zhash = gs.zhash;
	entry.bestmove = bestmove;
	entry.draft = curDepth;
	entry.score = bestscore;
	entry.nodeType = ttNode;

	tTable[gs.zhash%ttsize] = entry;
    }

    return bestscore;
}

int alphaBeta(GameState& gs, int alpha, int beta, int curDepth,
	int ply, const zobristKeys& zobrist) {
    // check for termination conditions
    if (nodes%2048 == 0) {
	if (movetime > 0) {
	    if (measureTimeSinceStart() >= movetime) {
		if constexpr (azalea::statistics)
		    outputStats("time termination in alphaBeta loop\n");
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
    if (inCheck) curDepth++;

    // check for 3 fold repetition
    if (checkThreeFoldRep(gs)) return 0;

    // check for transposition table hit
    const auto probeEntry = tTable[gs.zhash%(ttsize)];
    Move hashmove;
    bool tthit = false;
    if (gs.zhash == probeEntry.zhash and probeEntry.draft != -1) { // tthit
        // save transposition table move for move ordering
        hashmove = probeEntry.bestmove;
        tthit = true;

        // if tthit is from a deep enough search, return
        // the score corresponding to the correct node type
	if (probeEntry.draft >= curDepth) { // deep enough
	    if (probeEntry.nodeType == NodeType::PVNode) {
		return probeEntry.score;
	    }
	    if (probeEntry.nodeType == NodeType::AlphaNode
		    and probeEntry.score <= alpha) {
		return alpha;
	    }
	    if (probeEntry.nodeType == NodeType::BetaNode
		    and probeEntry.score >= beta) {
		return beta;
	    }
	}
    }

    // enter quiescence search
    if (curDepth <= 0) return qsearch(gs, alpha, beta, zobrist);

    nodes++;
    int bestscore = -INF;
    NodeType ttNode = NodeType::AlphaNode;
    Move bestmove;

    // if we have a tthit, check if ttmove is legal, if so -> sort to front
    if (tthit) {
        if (std::find(movelist.begin(), movelist.end(), hashmove)
        	    != movelist.end()) {
            movelist.erase(
		    std::remove(movelist.begin(), movelist.end(), hashmove),
        	    movelist.end());
            movelist.insert(movelist.begin(), hashmove);
        }
    }

    // move ordering
    auto scores = scoreMoves(gs, movelist, hashmove);
    sortMove(movelist, scores, 0);

    // ================== PVS first move ==================
    const auto umi = gs.makeMove(movelist[0], zobrist);
    const int score = -alphaBeta(gs, -beta, -alpha, curDepth-1, ply+1, zobrist);
    gs.unmakeMove(umi, zobrist);

    bestscore = score;
    bestmove = movelist[0];

    // beta cutoff
    if (score >= beta) {
	ttNode = NodeType::BetaNode;
	goto saveTTAndReturn;
    }

    // raising alpha
    if (score > alpha) {
	ttNode = NodeType::PVNode;
	alpha = score;
    }
    // ================== end of PVS first move ==================


    // iterate over remaining legal moves
    for (size_t n=1; n<movelist.size(); n++) {
	sortMove(movelist, scores, n);
	const auto& m = movelist[n];
	if (terminateSearch) break;

	const auto umi = gs.makeMove(m, zobrist);
	int score = -alphaBeta(gs, -alpha-1, -alpha,
			       curDepth-1, ply+1, zobrist);
	if (score > alpha and score < beta) {
	    // research needed, use full window
	    score = -alphaBeta(gs, -beta, -alpha, curDepth-1, ply+1, zobrist);
	    if (score > alpha) {
		ttNode = NodeType::PVNode;
		alpha = score;
	    }
	}
	gs.unmakeMove(umi, zobrist);

	if (score > bestscore) {
	    bestscore = score;
	    bestmove = m;
	}

	// beta cutoff
	if (score >= beta) {
	    ttNode = NodeType::BetaNode;
	    break;
	}

	// raising alpha
	if (score > alpha) {
	    ttNode = NodeType::PVNode;
	    alpha = score;
	}
    }

    // store search info in transposition table
saveTTAndReturn:
    if (not terminateSearch) {
	TTentry entry;
	entry.zhash = gs.zhash;
	entry.bestmove = bestmove;
	entry.draft = curDepth;
	entry.score = bestscore;
	entry.nodeType = ttNode;

	tTable[gs.zhash%ttsize] = entry;
    }

    return bestscore;
}