#include "search.hpp"
#include "movegeneration.hpp"
#include "eval.hpp"
#include "search_helper.hpp"
#include "movepicker.hpp"

#include <cmath>
#include <array>
#include <chrono>
#include <algorithm>

namespace azalea {

using ms = std::chrono::milliseconds;

std::array<std::array<int, 256>, MAX_PLY> gen_lmr_table() {
    std::array<std::array<int, 256>, MAX_PLY> table{};
    for (size_t depth=1; depth<MAX_PLY; depth++) {
        for (size_t moves=1; moves<256; moves++) {
            table[depth][moves] = int(0.75 + std::log(depth) * std::log(moves) / 2.25);
        }
    }
    return table;
}

constexpr std::array<std::size_t, MAX_PLY> gen_lmp_thresholds() {
    std::array<std::size_t, MAX_PLY> thrs;
    for (std::size_t i=0; i<MAX_PLY; ++i) thrs[i] = 3 + i * i;
    return thrs;
}

constexpr auto LMP_THRESHOLD = gen_lmp_thresholds();
const auto LMR_TABLE = gen_lmr_table();

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
 * Update the array for quiet move history heuristic using history gravity.
 */
void update_history(int& entry, int bonus) {
    entry += bonus - entry * std::abs(bonus) / MAX_HISTORY;
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
            state.seldepth = 0;
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
        si.seldepth = state.seldepth;
        si.score_cp = best_score;
        si.nodes = state.nodes;
        si.time = std::chrono::duration_cast<ms>(end - state.start);
        si.nps = state.nodes * 1000 / std::max<std::uint64_t>(static_cast<std::uint64_t>(si.time.count()), 1);
        si.hashfull = ttable.hashfull();
        si.pv = best_pv;
        reporter.on_info(si);

        // reached soft time limit?
        if (state.timed and state.elapsed() >= state.soft) break;
    }
    
    reporter.on_bestmove(best_move, best_pv.size() > 1 ? best_pv[1] : Move{});
}

/**
 * Actual search algorithm. Currently an implementation of a PVS (principle variation search)
 * with different pruning and reduction heuristics.
 */
int alpha_beta(Board& board, int depth, const int ply,
               int alpha, int beta, SearchState& state, TranspositionTable& ttable) {
    const bool is_pv_node = beta - alpha > 1;
    state.pv_length[ply] = ply; // empty PV at this node


    state.seldepth = std::max(ply, state.seldepth);

    if (state.should_stop()) return 0; // result gets discarded, score doesnt matter

    // 50 move rule and repetition
    if (ply > 0 and board.halfmove_clock() >= 100) return 0;
    if (ply > 0 and board.is_repetition()) return 0;


    // --------------------------- transposition table -------------------------------------
    const auto tt_entry = ttable.probe(board.hash());
    Move tt_move{};
    if (tt_entry != nullptr) {
        tt_move = tt_entry->move;
        if (not is_pv_node and ply > 0 and tt_entry->depth >= depth) {
            int tt_score = tt_entry->score;
            // adjust tt_score in case of mate
            if (tt_score >=  MATE_BOUND) tt_score -= ply;
            if (tt_score <= -MATE_BOUND) tt_score += ply;
            if (tt_entry->bound == TTentry::Bound::Exact) return tt_score;
            if (tt_entry->bound == TTentry::Bound::Lower and tt_score >= beta) return tt_score;
            if (tt_entry->bound == TTentry::Bound::Upper and tt_score <= alpha) return tt_score;
        }
    }

    if (depth <= 0) return quiescence(board, ply, alpha, beta, state);
    if (ply >= MAX_PLY - 1) return eval(board);


    // --------------------------- node level pruning -------------------------------------
    // reverse futility pruning
    const bool in_check = board.in_check();
    const int static_eval = in_check ? -SCORE_INF : eval(board);
    if (not is_pv_node and not in_check and depth <= 6 and std::abs(beta) < MATE_BOUND) {
        if (static_eval - 80*depth >= beta) {
            return static_eval - 80*depth; // fail soft
        }
    }

    // null move pruning
    if (not is_pv_node and not in_check and depth >= 3 and static_eval >= beta
            and board.player_has_non_pawn_material() and board.position_history.back().pliesFromLastNull > 0) { 
        const int reduction = 3 + depth / 4;
        board.make_null_move();
        int score = -alpha_beta(board, depth-1-reduction, ply+1, -beta, -beta+1, state, ttable);
        board.unmake_null_move();

        if (state.stopped) return 0;
        if (score >= beta) {
            return score >= MATE_BOUND ? beta : score; // dont report unproven mates
        }
    }

    // internal iterative reduction
    // if this node has no TT it is most likely not important if it turns out to be
    // important, the next iterative deepening loop will search it and a TT move will be present
    if (tt_move == Move{} and depth >= 4) depth -=1;

    // movepicker for TT, killers and MVV-LVA
    MovePicker picker(board, tt_move, state.killers[static_cast<std::size_t>(ply)], state.history);
    int moves_searched = 0;

    // info for history heuristic
    std::array<Move, 256> quiets_tried;
    std::size_t num_quiets = 0;

    // -------------------------------- loop over legal moves -----------------------------------
    const auto original_alpha = alpha;
    int best_score = -SCORE_INF;
    Move best_move{}, m;
    const int futility_margin = 100 + 100 * depth; // TODO: tune
    while ((m = picker.next()) != Move{}) {
        if (not board.is_legal(m)) continue;

        // late move pruning and futility pruning
        if (m.is_quiet() and not is_pv_node and not in_check and depth<=6 and best_score > -MATE_BOUND) {
            // LMP: if a quiet move if late within the order quiet list, it is unlikely to be relevant
            if (num_quiets>LMP_THRESHOLD[static_cast<std::size_t>(depth)])
                continue; // skip this late quiet move
            // futiilty pruning
            // if the static eval is already so bad, quiet moves are unlikely to rescue us
            if (static_eval + futility_margin <= alpha and not board.gives_check(m)) continue;
        }

        board.make_move(m);

        // side to move is now the opponent (after move is made)
        const bool giving_check = board.in_check();

        // -------------------------------- iterative search call -----------------------------
        // first move is searched with the full window in PVS
        int score;
        if (moves_searched == 0) {
            score = -alpha_beta(board, depth-1, ply+1, -beta, -alpha, state, ttable);

        // not the first move -> try zero window search
        } else {
            // late move reduction, first search with null window
            int lm_reduction = 0;
            if (depth >= 3 and moves_searched >= 3 and m.is_quiet() and not in_check and not giving_check) {
                lm_reduction = LMR_TABLE[static_cast<std::size_t>(depth)][static_cast<std::size_t>(moves_searched)];
                if (is_pv_node) lm_reduction -= 1; // dont reduce so aggresively in PV nodes
                lm_reduction = std::clamp(lm_reduction, 0, depth-2); // dont reduce into quiescense search
            }

            // try zero window search with LMR
            score = -alpha_beta(board, depth-1-lm_reduction, ply+1, -alpha-1, -alpha, state, ttable);

            // LM reduced search beat alpha, research at full depth (but zero window)
            if (score > alpha and lm_reduction > 0)
                score = -alpha_beta(board, depth-1, ply+1, -alpha-1, -alpha, state, ttable);

            // the proof has failed, get exact score
            // if the non-reduced search beats alpha to we are in a PV node
            if (score > alpha and score < beta)
                score = -alpha_beta(board, depth-1, ply+1, -beta, -alpha, state, ttable);
        }

        board.unmake_move(m);
        ++moves_searched;
        ++state.nodes;

        if (state.stopped) return 0;

        // found a better move? -> do bookeeping
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
                    // quiet move caused a beta cutoff
                    // -> update killer and history heuristic
                    if (best_move.is_quiet()) {
                        const int bonus = std::min(depth * depth * 16, 1024);
                        const auto us = static_cast<std::size_t>(board.white_to_move);

                        update_killers(state, ply, best_move);
                        update_history(state.history[us][best_move.from()][best_move.to()], bonus);

                        // penalize all quiet moves that were tried before this
                        // they did not result in a cutoff
                        for (std::size_t i=0; i<num_quiets; ++i) {
                            const Move qmove = quiets_tried[i];
                            update_history(state.history[us][qmove.from()][qmove.to()], -bonus);
                        }
                    }
                    break;
                } // beta cutoff
            } // raised alpha
        } // found better move
        
        // tried a quiet move and achieved no cutoff?
        // -> add to tried quiets
        if (m.is_quiet()) quiets_tried[num_quiets++] = m;
    } // loop over moves to be searched

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
    state.seldepth = std::max(ply, state.seldepth);

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

    // movepicker for TT and MVV-LVA
    MovePicker picker = in_check ? MovePicker(board, Move{}, std::array<Move, 2>{}, std::array<std::array<std::array<int, 64>, 64>, 2>{})
                                 : MovePicker(board, Move{});
    int moves_searched = 0;

    // loop over legal moves
    Move m;
    while ((m = picker.next()) != Move{}) {
        if (not board.is_legal(m)) continue;
        ++moves_searched;

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
    if (moves_searched == 0 and in_check) return -SCORE_MATE + ply; // mate

    return best_score;
}

} // namespace azalea
