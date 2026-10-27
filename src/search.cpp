#include "search.hpp"
#include "movegeneration.hpp"
#include "eval.hpp"
#include "search_helper.hpp"

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

    int max_depth = limits.depth > 0 ? limits.depth : MAX_PLY - 1;
    max_depth = std::min(max_depth, MAX_PLY - 1);


    // -------------------------- iterative deepening ---------------------------
    Move best_move;
    int  best_score = 0;
    std::vector<Move> best_pv;
    for (int depth=1; depth<=max_depth; ++depth) {

        // call search on root node
        const auto score = alpha_beta(board, depth, 0, -SCORE_INF, SCORE_INF, state);

        // discard incomplete iterations
        if (state.stopped and depth > 1) break;

        // copy row 0 from the PV table before the next iteration overwrites it
        const auto& row = state.pv_table[0];
        std::vector<Move> pv(row, row + state.pv_length[0]);

        if (not pv.empty()) {
            best_move = pv.front();
            best_score = score;
            best_pv = std::move(pv);
            state.root_best = best_move; // TODO: delete when TT is implemented
        }

        const auto end = std::chrono::steady_clock::now();

        SearchInfo si;
        si.depth = depth;
        si.score_cp = best_score;
        si.nodes = state.nodes;
        si.time = std::chrono::duration_cast<ms>(end - state.start);
        si.pv = best_pv;
        reporter.on_info(si);

        // reached soft time limit?
        if (state.timed and state.elapsed() >= state.soft) break;
    }
    
    reporter.on_bestmove(best_move, best_pv.size() > 1 ? best_pv[1] : Move{});
}

int alpha_beta(Board& board, const int depth, const int ply,
               int alpha, int beta, SearchState& state) {
    state.pv_length[ply] = ply; // empty PV at this node

    if (depth <= 0) return quiescence(board, ply, alpha, beta, state);
    if (ply >= MAX_PLY - 1) return eval(board);

    if (state.should_stop()) return 0; // result gets discarded, score doesnt matter

    // generate all legal moves
    Movelist mlist;
    generate_legal_moves(board, mlist);

    // no legal moves?
    if (mlist.size() == 0) {
        if (board.in_check()) return -SCORE_MATE + ply; // mate
        else return 0; // stalemate
    }

    // 50 move rule and repetition
    if (ply > 0 and board.halfmove_clock() >= 100) return 0;
    if (ply > 0 and board.is_repetition()) return 0;

    // order moves, hint-move first, then MVV-LVA
    Move hint{};
    if (ply == 0 and state.root_best != Move{}) hint = state.root_best;
    int scores[256];
    for (std::size_t i=0; i<mlist.size(); ++i) scores[i] = score_move_mvv_lva(board, mlist.moves[i], hint);

    // loop over legal moves
    int best_score = -SCORE_INF;
    for (size_t i=0; i<mlist.size(); ++i) {
        // find most promising move remaining in the movelist
        size_t promising = i;
        for (size_t j=i+1; j<mlist.size(); ++j)
            if (scores[j] > scores[promising]) promising = j;
        std::swap(mlist.moves[i], mlist.moves[promising]);
        std::swap(scores[i], scores[promising]);

        const Move& m = mlist.moves[i];

        ++state.nodes;
        Board copy = board;
        copy.make_move(m);
        int score = -alpha_beta(copy, depth-1, ply+1, -beta, -alpha, state);

        if (score > best_score) {
            best_score = score;

            if (score > alpha) {
                alpha = score;

                // new PV found: m followed by childs PV
                state.pv_table[ply][ply] = m;
                // copy childs PV
                for (int k=ply+1; k<state.pv_length[ply+1]; ++k)
                    state.pv_table[ply][k] = state.pv_table[ply+1][k];
                // update PV length
                state.pv_length[ply] = state.pv_length[ply+1];

                if (alpha >= beta) break; // beta cutoff
            }
        }
    }

    return best_score;
}

int quiescence(Board& board, const int ply, int alpha, int beta, SearchState& state) {
    const bool in_check = board.in_check();
    int best_score = -SCORE_INF;
    if (not in_check) {
        const int stand_pat = eval(board);
        if (stand_pat >= beta) return stand_pat; // already good enough, no capture needed
        if (stand_pat > alpha) alpha = stand_pat;
        best_score = stand_pat;
    }

    if (ply >= MAX_PLY - 1) return eval(board);
    if (state.should_stop()) return eval(board);

    if (ply > 0 and board.is_repetition()) return 0;

    // generate all legal moves
    Movelist mlist;
    generate_legal_moves(board, mlist);

    // no legal moves?
    if (mlist.size() == 0) {
        if (board.in_check()) return -SCORE_MATE + ply; // mate
        else return 0; // stalemate
    }

    // MVV-LVA
    Move hint{};
    int scores[256];
    for (std::size_t i=0; i<mlist.size(); ++i) scores[i] = score_move_mvv_lva(board, mlist.moves[i], hint);

    // loop over legal moves
    for (size_t i=0; i<mlist.size(); ++i) {
        // find most promising move remaining in the movelist
        size_t promising = i;
        for (size_t j=i+1; j<mlist.size(); ++j)
            if (scores[j] > scores[promising]) promising = j;
        std::swap(mlist.moves[i], mlist.moves[promising]);
        std::swap(scores[i], scores[promising]);

        const Move& m = mlist.moves[i];
        // dont search quiet moves
        if (not in_check and not (m.is_capture() or m.is_queen_promotion())) break;

        ++state.nodes;
        Board copy = board;
        copy.make_move(m);
        int score = -quiescence(copy, ply+1, -beta, -alpha, state);

        if (score > best_score) {
            best_score = score;
            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) break; // beta cutoff
            }
        }
    }

    return best_score;
}

} // namespace azalea
