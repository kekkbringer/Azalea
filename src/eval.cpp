#include "eval.hpp"
#include "util.hpp"

#include "eval_trace.hpp" // for tuning

#include <bit>
#include <algorithm>

namespace azalea {

/**
 * Evaluates the board in centipawns from the perspective of the side to move.
 */
template<bool Trace>
int eval_template(const Board& b, EvalTrace* t) {
    // determine game phase
    int material_score = PAWN_MATERIAL   * std::popcount(b.pawns)
                       + KNIGHT_MATERIAL * std::popcount(b.knights)
                       + BISHOP_MATERIAL * std::popcount(b.bishops)
                       + ROOK_MATERIAL   * std::popcount(b.rooks)
                       + QUEEN_MATERIAL  * std::popcount(b.queens);
    constexpr int max_material = 16 * PAWN_MATERIAL
                               +  4 * KNIGHT_MATERIAL
                               +  4 * BISHOP_MATERIAL
                               +  4 * ROOK_MATERIAL
                               +  2 * QUEEN_MATERIAL;
    material_score = std::min(material_score, max_material);
    if constexpr (Trace) t->phase = material_score;

    // give a bonus for the side to move in the early game
    int early_score = b.white_to_move ? TEMPO_EVAL : -TEMPO_EVAL;
    if constexpr (Trace) {
        if (b.white_to_move) t->early[TEMPO]++;
        else t->early[TEMPO]--;
    }
    int late_score = 0;

    // piece value and placement
    auto wp = b.white_pieces & b.pawns;
    auto wn = b.white_pieces & b.knights;
    auto wb = b.white_pieces & b.bishops;
    auto wr = b.white_pieces & b.rooks;
    auto wq = b.white_pieces & b.queens;
    auto wk = b.white_pieces & b.kings;
    const auto wk_idx = static_cast<std::size_t>(lsb(wk));
    while (wp != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(wp));
        early_score += EARLY_PAWN_EVAL + EARLY_PAWN_SQ[sq];
        late_score  += LATE_PAWN_EVAL  + LATE_PAWN_SQ[sq];
        if constexpr (Trace) {
            t->early[PAWN_EVAL]++;
            t->late[PAWN_EVAL]++;
            t->early[PST_START + sq]++;
            t->late[PST_START + sq]++;
        }
    }
    while (wn != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(wn));
        early_score += EARLY_KNIGHT_EVAL + EARLY_KNIGHT_SQ[sq];
        late_score  += LATE_KNIGHT_EVAL  + LATE_KNIGHT_SQ[sq];
        if constexpr (Trace) {
            t->early[KNIGHT_EVAL]++;
            t->late[KNIGHT_EVAL]++;
            t->early[PST_START + 1*64 + sq]++;
            t->late[PST_START + 1*64 + sq]++;
        }
    }
    while (wb != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(wb));
        early_score += EARLY_BISHOP_EVAL + EARLY_BISHOP_SQ[sq];
        late_score  += LATE_BISHOP_EVAL  + LATE_BISHOP_SQ[sq];
        if constexpr (Trace) {
            t->early[BISHOP_EVAL]++;
            t->late[BISHOP_EVAL]++;
            t->early[PST_START + 2*64 + sq]++;
            t->late[PST_START + 2*64 + sq]++;
        }
    }
    while (wr != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(wr));
        early_score += EARLY_ROOK_EVAL + EARLY_ROOK_SQ[sq];
        late_score  += LATE_ROOK_EVAL  + LATE_ROOK_SQ[sq];
        if constexpr (Trace) {
            t->early[ROOK_EVAL]++;
            t->late[ROOK_EVAL]++;
            t->early[PST_START + 3*64 + sq]++;
            t->late[PST_START + 3*64 + sq]++;
        }
    }
    while (wq != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(wq));
        early_score += EARLY_QUEEN_EVAL + EARLY_QUEEN_SQ[sq];
        late_score  += LATE_QUEEN_EVAL  + LATE_QUEEN_SQ[sq];
        if constexpr (Trace) {
            t->early[QUEEN_EVAL]++;
            t->late[QUEEN_EVAL]++;
            t->early[PST_START + 4*64 + sq]++;
            t->late[PST_START + 4*64 + sq]++;
        }
    }
    early_score += EARLY_KING_SQ[wk_idx];
    late_score += LATE_KING_SQ[wk_idx];
    if constexpr (Trace) {
        t->early[PST_START + 5*64 + wk_idx]++;
        t->late[PST_START + 5*64 + wk_idx]++;
    }

    auto bp = b.black_pieces & b.pawns;
    auto bn = b.black_pieces & b.knights;
    auto bb = b.black_pieces & b.bishops;
    auto br = b.black_pieces & b.rooks;
    auto bq = b.black_pieces & b.queens;
    auto bk = b.black_pieces & b.kings;
    const auto bk_idx = static_cast<std::size_t>(lsb(bk));
    while (bp != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(bp));
        early_score -= EARLY_PAWN_EVAL + EARLY_PAWN_SQ[sq^56];
        late_score  -= LATE_PAWN_EVAL  + LATE_PAWN_SQ[sq^56];
        if constexpr (Trace) {
            t->early[PAWN_EVAL]--;
            t->late[PAWN_EVAL]--;
            t->early[PST_START + (sq^56)]--;
            t->late[PST_START + (sq^56)]--;
        }
    }
    while (bn != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(bn));
        early_score -= EARLY_KNIGHT_EVAL + EARLY_KNIGHT_SQ[sq^56];
        late_score  -= LATE_KNIGHT_EVAL  + LATE_KNIGHT_SQ[sq^56];
        if constexpr (Trace) {
            t->early[KNIGHT_EVAL]--;
            t->late[KNIGHT_EVAL]--;
            t->early[PST_START + 1*64 + (sq^56)]--;
            t->late[PST_START + 1*64 + (sq^56)]--;
        }
    }
    while (bb != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(bb));
        early_score -= EARLY_BISHOP_EVAL + EARLY_BISHOP_SQ[sq^56];
        late_score  -= LATE_BISHOP_EVAL  + LATE_BISHOP_SQ[sq^56];
        if constexpr (Trace) {
            t->early[BISHOP_EVAL]--;
            t->late[BISHOP_EVAL]--;
            t->early[PST_START + 2*64 + (sq^56)]--;
            t->late[PST_START + 2*64 + (sq^56)]--;
        }
    }
    while (br != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(br));
        early_score -= EARLY_ROOK_EVAL + EARLY_ROOK_SQ[sq^56];
        late_score  -= LATE_ROOK_EVAL  + LATE_ROOK_SQ[sq^56];
        if constexpr (Trace) {
            t->early[ROOK_EVAL]--;
            t->late[ROOK_EVAL]--;
            t->early[PST_START + 3*64 + (sq^56)]--;
            t->late[PST_START + 3*64 + (sq^56)]--;
        }
    }
    while (bq != 0) {
        auto sq = static_cast<std::size_t>(pop_lsb(bq));
        early_score -= EARLY_QUEEN_EVAL  + EARLY_QUEEN_SQ[sq^56];
        late_score -= LATE_QUEEN_EVAL  + LATE_QUEEN_SQ[sq^56];
        if constexpr (Trace) {
            t->early[QUEEN_EVAL]--;
            t->late[QUEEN_EVAL]--;
            t->early[PST_START + 4*64 + (sq^56)]--;
            t->late[PST_START + 4*64 + (sq^56)]--;
        }
    }
    early_score -= EARLY_KING_SQ[bk_idx^56];
    late_score -= LATE_KING_SQ[bk_idx^56];
    if constexpr (Trace) {
        t->early[PST_START + 5*64 + (bk_idx^56)]--;
        t->late[PST_START + 5*64 + (bk_idx^56)]--;
    }

    // passed pawns
    wp = b.white_pieces & b.pawns;
    bp = b.black_pieces & b.pawns;
    while (wp != 0) {
        const auto sq = static_cast<std::size_t>(pop_lsb(wp));
        if ((PASSED_PAWN_MASK_TABLE[sq] & bp) == 0) {
            early_score += EARLY_PASSED_PAWN[sq/8];
            late_score  +=  LATE_PASSED_PAWN[sq/8];
            late_score += PASSED_PAWN_KING_DISTANCE_EVAL[static_cast<std::size_t>(DISTANCE_TABLE[sq][wk_idx])]
                        - PASSED_PAWN_KING_DISTANCE_EVAL[static_cast<std::size_t>(DISTANCE_TABLE[sq][bk_idx])];
            if constexpr (Trace) {
                t->early[PASSED_PAWN_START + sq/8]++;
                t->late[PASSED_PAWN_START + sq/8]++;
                t->late[PASSED_PAWN_KING_DISTANCE + static_cast<std::size_t>(DISTANCE_TABLE[sq][wk_idx])]++;
                t->late[PASSED_PAWN_KING_DISTANCE + static_cast<std::size_t>(DISTANCE_TABLE[sq][bk_idx])]--;
            }
        }
    }
    wp = b.white_pieces & b.pawns;
    bp = b.black_pieces & b.pawns;
    while (bp != 0) {
        const auto sq = static_cast<std::size_t>(pop_lsb(bp));
        if ((__builtin_bswap64(PASSED_PAWN_MASK_TABLE[sq^56]) & wp) == 0) {
            early_score -= EARLY_PASSED_PAWN[(sq^56)/8];
            late_score  -=  LATE_PASSED_PAWN[(sq^56)/8];
            late_score += PASSED_PAWN_KING_DISTANCE_EVAL[static_cast<std::size_t>(DISTANCE_TABLE[sq][wk_idx])]
                        - PASSED_PAWN_KING_DISTANCE_EVAL[static_cast<std::size_t>(DISTANCE_TABLE[sq][bk_idx])];
            if constexpr (Trace) {
                t->early[PASSED_PAWN_START + (sq^56)/8]--;
                t->late[PASSED_PAWN_START + (sq^56)/8]--;
                t->late[PASSED_PAWN_KING_DISTANCE + static_cast<std::size_t>(DISTANCE_TABLE[sq][wk_idx])]++;
                t->late[PASSED_PAWN_KING_DISTANCE + static_cast<std::size_t>(DISTANCE_TABLE[sq][bk_idx])]--;
            }
        }
    }

    // bishop pair
    if (std::popcount(b.white_pieces & b.bishops) > 1) {
        early_score += EARLY_BISHOP_PAIR_EVAL;
        late_score  += LATE_BISHOP_PAIR_EVAL;
        if constexpr (Trace) {
            t->early[BISHOP_PAIR]++;
            t->late[BISHOP_PAIR]++;
        }
    }
    if (std::popcount(b.black_pieces & b.bishops) > 1) {
        early_score -= EARLY_BISHOP_PAIR_EVAL;
        late_score  -= LATE_BISHOP_PAIR_EVAL;
        if constexpr (Trace) {
            t->early[BISHOP_PAIR]--;
            t->late[BISHOP_PAIR]--;
        }
    }


    // taper score between early and late game
    int tapered_score = (early_score * material_score + late_score * (max_material - material_score)) / max_material;

    return b.white_to_move ? tapered_score : -tapered_score;
}

int eval(const Board& b) { return eval_template<false>(b, nullptr); }
void trace_eval(const Board& b, EvalTrace& t) { eval_template<true>(b, &t); }

} // namespace azalea
