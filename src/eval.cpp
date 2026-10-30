#include "eval.hpp"
#include "attacks.hpp"
#include "util.hpp"
#include "weights.hpp"

#include "eval_trace.hpp" // for tuning

#include <bit>
#include <algorithm>

namespace azalea {

/**
 * Adds a term to the score and records it in the trace.
 */
template<bool White, bool Trace>
inline void add(Score& s, EvalTrace* t, int idx, int count=1) {
    s += WEIGHTS[static_cast<std::size_t>(idx)] * count;
    if constexpr (Trace) {
        constexpr int sign = White ? 1 : -1;
        t->early[idx] += sign * count;
        t->late[idx]  += sign * count;
    }
}

/**
 * Adds a term to the LATE GAME SCORE ONLY and records it in the trace.
 */
template<bool White, bool Trace>
inline void add_late(Score& s, EvalTrace* t, int idx, int count=1) {
    s.late += WEIGHTS[static_cast<std::size_t>(idx)].late * count;
    if constexpr (Trace) {
        constexpr int sign = White ? 1 : -1;
        t->late[idx]  += sign * count;
    }
}

template<bool White>
constexpr Bitboard passed_mask(int sq) {
    return White ? PASSED_PAWN_MASK_TABLE[static_cast<std::size_t>(sq)]
                 : __builtin_bswap64(PASSED_PAWN_MASK_TABLE[static_cast<std::size_t>(sq)^56]);
}

struct EvalInfo {
    enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

    Bitboard attacked_by[2][6]{};
    Bitboard attacked[2]{};
    Bitboard attacked_twice[2]{};

    void record(int c, int pt, Bitboard atk) {
        attacked_twice[c]  |= attacked[c] & atk;
        attacked[c]        |= atk;
        attacked_by[c][pt] |= atk;
    }
};

template<bool White, bool Trace>
Score eval_pawns(const Board& b, EvalTrace* t) {
    const Bitboard own   = White ? b.white_pieces : b.black_pieces;
    const Bitboard enemy = White ? b.black_pieces : b.white_pieces;
    const Bitboard own_pawns   = own   & b.pawns;
    const Bitboard enemy_pawns = enemy & b.pawns;
    const int own_king_idx = lsb(own & b.kings);
    const int enemy_king_idx = lsb(enemy & b.kings);

    Score s;
    for (Bitboard bb = own_pawns; bb != 0;) {
        const int sq = pop_lsb(bb);
        const int rel_rank = White ? sq/8 : (sq^56)/8; // 0 is own backrank

        // passed pawn, plus king distance to passer
        if ((passed_mask<White>(sq) & enemy_pawns) == 0) {
            add<White, Trace>(s, t, PASSED_PAWN_START + rel_rank);
            add_late<White, Trace>(s, t, PASSED_PAWN_KING_DISTANCE + DISTANCE_TABLE[static_cast<std::size_t>(sq)][static_cast<std::size_t>(own_king_idx)]);
            add_late<White, Trace>(s, t, PASSED_PAWN_KING_DISTANCE + DISTANCE_TABLE[static_cast<std::size_t>(sq)][static_cast<std::size_t>(enemy_king_idx)], -1);
        }

        // doubled or tripled pawns
        if (more_than_one(FILES[sq%8] & own_pawns))
            add<White, Trace>(s, t, DOUBLED_PAWN);

        // isolated pawn
        if ((ADJACENT_FILES[sq%8] & own_pawns) == 0)
            add<White, Trace>(s, t, ISOLATED_PAWN);
    }
    return s;
}

template<bool White, bool Trace>
Score eval_pieces(const Board& b, EvalInfo& ei, EvalTrace* t) {
    constexpr int us = White ? 0 : 1;
    constexpr int them = 1 - us;

    const Bitboard own         = White ? b.white_pieces : b.black_pieces;
    const Bitboard enemy       = White ? b.black_pieces : b.white_pieces;
    const Bitboard occ         = b.white_pieces | b.black_pieces;
    const Bitboard own_pawns   = own & b.pawns;
    const Bitboard enemy_pawns = enemy & b.pawns;

    // square that count for mobility
    // not blocked by own pawns or king (they move rarely)
    // not attacked by enemy pawns
    const Bitboard mobility_area
        = ~(own_pawns | (own & b.kings) | ei.attacked_by[them][EvalInfo::PAWN]);

    Score s;

    // knight
    for (Bitboard bb = own & b.knights; bb != 0;) {
        const int sq = pop_lsb(bb);
        const Bitboard atk = attacks::knight(sq);
        ei.record(us, EvalInfo::KNIGHT, atk);
        add<White, Trace>(s, t, KNIGHT_MOBILITY + std::popcount(atk & mobility_area));
    }

    // bishop
    for (Bitboard bb = own & b.bishops; bb != 0;) {
        const int sq = pop_lsb(bb);
        const Bitboard atk = attacks::bishop(sq, occ);
        ei.record(us, EvalInfo::BISHOP, atk);
        add<White, Trace>(s, t, BISHOP_MOBILITY + std::popcount(atk & mobility_area));
    }
    // bishop pair
    if (more_than_one(own & b.bishops)) add<White, Trace>(s, t, BISHOP_PAIR);

    // rook
    for (Bitboard bb = own & b.rooks; bb != 0;) {
        const int sq = pop_lsb(bb);
        const Bitboard atk = attacks::rook(sq, occ);
        ei.record(us, EvalInfo::ROOK, atk);
        add<White, Trace>(s, t, ROOK_MOBILITY + std::popcount(atk & mobility_area));

        const Bitboard file = FILES[sq % 8];
        if (not (file & own_pawns)) {
            add<White, Trace>(s, t, (file&enemy_pawns) ? ROOK_HALF_OPEN : ROOK_OPEN);
        }
    }

    // queen
    for (Bitboard bb = own & b.queens; bb != 0;) {
        const int sq = pop_lsb(bb);
        const Bitboard atk = attacks::queen(sq, occ);
        ei.record(us, EvalInfo::QUEEN, atk);
        add<White, Trace>(s, t, QUEEN_MOBILITY + std::popcount(atk & mobility_area));
    }

    return s;
}

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


    EvalInfo ei;

    // First, the pawn and king attacks need to be calculated
    // they are needed for the piece mobility evaluation
    ei.record(0, EvalInfo::PAWN, attacks::pawn_white(b.white_pieces & b.pawns));
    ei.record(1, EvalInfo::PAWN, attacks::pawn_black(b.black_pieces & b.pawns));
    ei.record(0, EvalInfo::KING, attacks::king(lsb(b.white_pieces & b.kings)));
    ei.record(1, EvalInfo::KING, attacks::king(lsb(b.black_pieces & b.kings)));

    Score s;

    // give a bonus for the side to move
    add<true, Trace>(s, t, TEMPO, b.white_to_move ? 1 : -1);

    // get material and piece table eval from board
    s += b.position_history.back().score;

    // eval mobility and other piece properties
    // (rook on open files, bishop pair, ...)
    s += eval_pieces<true, Trace>(b, ei, t) - eval_pieces<false, Trace>(b, ei, t);
    s += eval_pawns<true, Trace>(b, t) - eval_pawns<false, Trace>(b, t);

    // TODO: king safety and other stuff


    // taper score between early and late game
    int tapered_score = (s.early * material_score + s.late * (max_material - material_score)) / max_material;

    return b.white_to_move ? tapered_score : -tapered_score;
}


int eval(const Board& b) { return eval_template<false>(b, nullptr); }
void trace_eval(const Board& b, EvalTrace& t) { eval_template<true>(b, &t); }

} // namespace azalea
