#include "search.hpp"
#include "movegeneration.hpp"
#include "eval.hpp"
#include "search_helper.hpp"
#include "movepicker.hpp"

#include <chrono>
#include <algorithm>

namespace azalea {

using ms = std::chrono::milliseconds;

/**
 * Keep two killer moves per ply, to add a new move the first is shifted to the
 * second slot and the new move is inserted in the first position.
 */
void update_killers(SearchState& state, const int ply, const Move move) {
    auto& k = state.killers[static_cast<std::size_t>(ply)];
    if (k[0] != move) { // dont add if the move is already here
        k[1] = k[0];
        k[0] = move;
    }
}

/**
 * Main search driver with an iterative deeping loop that calls a principle variation
 * search with exponentially widening aspiration windows.
 */
void search(Board& board, const SearchLimits& limits, SearchReporter& reporter, TranspositionTable& ttable, std::stop_token st) {
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
        // determine current alpha and beta
        int alpha, beta, delta;
        // if the search is shallow or mate is found search the full window
        if (depth < 5 or std::abs(best_score) >= MATE_BOUND) {
            alpha = -SCORE_INF;
            beta  =  SCORE_INF;
            delta =  SCORE_INF; // windoe is already full, doesnt need to be widened
        // otherwise start with 0.5 pawn window centered around score
        } else {
            delta = 25; // centipawns to the left and right -> 0.5 pawn window
            alpha = std::max(best_score - delta, -SCORE_INF);
            beta  = std::min(best_score + delta,  SCORE_INF);
        }

        // call search on root node, might need to restart to widen the window
        // ----------------------- aspiration window loop ----------------------------
        int score;
        while (true) {
            score = alpha_beta(board, depth, 0, alpha, beta, state, ttable);
            if (state.stopped) break;

            if (score <= alpha) { // fail low -> true score is lower
                beta = (alpha + beta) / 2; // this might not be optimal
                alpha = std::max(score - delta, -SCORE_INF);
            } else if (score >= beta) { // fail high -> true score is higher
                beta = std::min(score + delta, SCORE_INF);
            } else break; // inside the window, score is exact

            // widen window exponentially
            delta += delta / 2;
        }

        // discard incomplete iterations
        if (state.stopped and depth > 1) break;

        // copy row 0 from the PV table before the next iteration overwrites it
        const auto& row = state.pv_table[0];
        std::vector<Move> pv(row, row + state.pv_length[0]);

        if (not pv.empty()) {
            best_move = pv.front();
            best_score = score;
            best_pv = std::move(pv);
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
               int alpha, int beta, SearchState& state, TranspositionTable& ttable) {
    const bool is_pv_node = beta - alpha > 1;
    state.pv_length[ply] = ply; // empty PV at this node

    // 50 move rule and repetition
    if (ply > 0 and board.halfmove_clock() >= 100) return 0;
    if (ply > 0 and board.is_repetition()) return 0;


    // look in transposition table
    const auto tt_entry = ttable.probe(board.hash());
    Move tt_move{};
    if (tt_entry != nullptr) {
        tt_move = tt_entry->move;
        if (ply > 0 and tt_entry->depth >= depth) {
            int tt_score = tt_entry->score;
            // adjust tt_score in case of mate
            if (tt_score >=  MATE_BOUND) tt_score -= ply;
            if (tt_score <= -MATE_BOUND) tt_score += ply;
            if (tt_entry->bound == TTentry::Bound::Exact) return tt_score;
            if (tt_entry->bound == TTentry::Bound::Lower and tt_score >= beta) return tt_score;
            if (tt_entry->bound == TTentry::Bound::Upper and tt_score <= alpha) return tt_score;
        }
    }
    const auto original_alpha = alpha;

    if (depth <= 0) return quiescence(board, ply, alpha, beta, state);
    if (ply >= MAX_PLY - 1) return eval(board);

    if (state.should_stop()) return 0; // result gets discarded, score doesnt matter

    // movepicker for TT, killers and MVV-LVA
    MovePicker picker(board, tt_move, state.killers[static_cast<std::size_t>(ply)]);
    int moves_searched = 0;

    // loop over legal moves
    int best_score = -SCORE_INF;
    Move best_move{}, m;
    while ((m = picker.next()) != Move{}) {
        if (not board.is_legal(m)) continue;
        board.make_move(m);

        // first move is searched with the full window in PVS
        int score;
        if (moves_searched == 0) {
            score = -alpha_beta(board, depth-1, ply+1, -beta, -alpha, state, ttable);

        // not the first move -> try zero window search
        } else {
            score = -alpha_beta(board, depth-1, ply+1, -alpha-1, -alpha, state, ttable);
            // the proof has failed, get exact score
            if (score > alpha and score < beta)
                score = -alpha_beta(board, depth-1, ply+1, -beta, -alpha, state, ttable);
        }

        board.unmake_move(m);
        ++moves_searched;
        ++state.nodes;

        if (score > best_score) {
            best_score = score;
            best_move = m;

            if (score > alpha) {
                alpha = score;

                if (is_pv_node) {
                    // new PV found: m followed by childs PV
                    state.pv_table[ply][ply] = m;
                    // copy childs PV
                    for (int k=ply+1; k<state.pv_length[ply+1]; ++k)
                        state.pv_table[ply][k] = state.pv_table[ply+1][k];
                    // update PV length
                    state.pv_length[ply] = state.pv_length[ply+1];
                }

                // beta cutoff
                if (alpha >= beta) {
                    if (not best_move.is_capture() and not m.is_promotion()) {
                        update_killers(state, ply, best_move);
                    }
                    break;
                }
            }
        }
    }

    // no legal moves?
    if (moves_searched == 0) {
        if (board.in_check()) return -SCORE_MATE + ply; // mate
        else return 0; // stalemate
    }

    // store information in transposition table
    if (not state.stopped) {
        // figure out bound
        TTentry::Bound bound = TTentry::Bound::Exact;
        if (best_score <= original_alpha) bound = TTentry::Bound::Upper;
        if (best_score >= beta)           bound = TTentry::Bound::Lower;

        // for an upper bound, keep the old move in the table
        if (bound == TTentry::Bound::Upper) best_move = Move{};

        // adjust for mate if needed
        int store_score = best_score;
        if (store_score >=  MATE_BOUND) store_score += ply;
        if (store_score <= -MATE_BOUND) store_score -= ply;

        // store
        ttable.store(board.hash(), best_move, store_score, depth, bound);
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

    if (board.is_repetition()) return 0;

    // movepicker for TT and MVV-LVA
    MovePicker picker(board, Move{});
    int moves_searched = 0;

    // loop over legal moves
    Move m;
    while ((m = picker.next()) != Move{}) {
        if (not board.is_legal(m)) continue;
        ++moves_searched;

        // dont search quiet moves
        if (not in_check and not (m.is_capture() or m.is_queen_promotion())) break;

        ++state.nodes;
        board.make_move(m);
        int score = -quiescence(board, ply+1, -beta, -alpha, state);
        board.unmake_move(m);

        if (score > best_score) {
            best_score = score;
            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) break; // beta cutoff
            }
        }
    }

    // no legal moves?
    if (moves_searched == 0 and board.in_check()) return -SCORE_MATE + ply; // mate

    return best_score;
}

} // namespace azalea
