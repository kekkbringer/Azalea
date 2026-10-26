#include "search.hpp"
#include "movegeneration.hpp"
#include "eval.hpp"

#include <chrono>
#include <algorithm>

namespace azalea {

using ms = std::chrono::milliseconds;

void search(Board& board, const SearchLimits& limits, SearchReporter& reporter, std::stop_token st) {
    SearchState state;
    state.st = st;
    state.start = std::chrono::steady_clock::now();
    state.node_limit = limits.nodes;

    // rudimentary time management
    // if the soft threshold is reached after an iteration, a new one is not started.
    // if the hard threshold is reached, the search is immediately terminated.
    constexpr ms overhead{30};
    const ms time = board.white_to_move ? limits.wtime : limits.btime;
    const ms inc  = board.white_to_move ? limits.winc  : limits.binc;

    state.timed = false;
    if (limits.movetime > ms{0}) { // use exact movetime
        state.timed = true;
        state.soft = state.hard = std::max(limits.movetime - overhead, ms{1});
    } else if (time > ms{0}) { // timed
        state.timed = true;
        const int moves_left = limits.moves_to_go > 0 ? limits.moves_to_go : 30;
        const ms available = std::max(time - overhead, ms{1});
        state.soft = std::min(time / (moves_left + 1) + inc * 3/4, available);
        state.hard = std::min(state.soft*3, available);
    }

    const int max_depth = limits.depth > 0 ? limits.depth : MAX_PLY;


    // -------------------------- iterative deepening ---------------------------
    Move best_move;
    int  best_score = 0;
    for (int depth=1; depth<=max_depth; ++depth) {

        // call search on root node
        const auto [move, score] = search_root(board, depth, state);

        // discard incomplete iterations
        if (state.stopped and depth > 1) break;

        best_move = move;
        best_score = score;

        const auto end = std::chrono::steady_clock::now();

        SearchInfo si;
        si.depth = depth;
        si.score_cp = best_score;
        si.nodes = state.nodes;
        si.time = std::chrono::duration_cast<ms>(end - state.start);
        reporter.on_info(si);

        // reached soft time limit?
        if (state.timed and state.elapsed() >= state.soft) break;
    }
    
    // "go infinite": the answer may only come after "stop"
    while (limits.infinite and not st.stop_requested())
        std::this_thread::sleep_for(ms(1));

    reporter.on_bestmove(best_move, Move{});
}

std::pair<Move, int> search_root(const Board& board, const int depth, SearchState& state) {
    int best_score = -SCORE_INF;
    Move best_move{};

    // generate all legal moves
    Movelist mlist;
    generate_legal_moves(board, mlist);

    // loop over legal moves
    for (const Move m: mlist) {
        ++state.nodes;
        Board copy = board;
        copy.make_move(m);
        int score = -negamax(copy, depth-1, 1, state);

        // found better move? -> save
        if (score > best_score) {
            best_score = score;
            best_move = m;
        }
    }

    return std::make_pair(best_move, best_score);
}

int negamax(const Board& board, const int depth, const int ply, SearchState& state) {
    if (depth <= 0) return eval(board);

    if (state.should_stop()) return 0;

    // generate all legal moves
    Movelist mlist;
    generate_legal_moves(board, mlist);

    // no legal moves?
    if (mlist.size() == 0) {
        if (board.in_check()) return -SCORE_MATE + ply; // mate
        else return 0; // stalemate
    }

    // loop over legal moves
    int best_score = -SCORE_INF;
    for (const Move m: mlist) {
        ++state.nodes;
        Board copy = board;
        copy.make_move(m);
        int score = -negamax(copy, depth-1, ply+1, state);
        best_score = std::max(best_score, score);
    }

    return best_score;
}

} // namespace azalea
