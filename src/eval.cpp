#include "eval.hpp"
#include "util.hpp"

#include <bit>
#include <algorithm>

namespace azalea {

/**
 * Evaluates the board in centipawns from the perspective of the side to move.
 */
int eval(const Board& b) {
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

    int early_score = b.white_to_move ? TEMPO_EVAL : -TEMPO_EVAL;
    int late_score = 0;

    auto wp = b.white_pieces & b.pawns;
    auto wn = b.white_pieces & b.knights;
    auto wb = b.white_pieces & b.bishops;
    auto wr = b.white_pieces & b.rooks;
    auto wq = b.white_pieces & b.queens;
    auto wk = b.white_pieces & b.kings;
    while (wp != 0) { auto sq = static_cast<std::size_t>(pop_lsb(wp)); early_score += EARLY_PAWN_EVAL   + EARLY_PAWN_SQ[sq];   late_score += LATE_PAWN_EVAL   + LATE_PAWN_SQ[sq]; }
    while (wn != 0) { auto sq = static_cast<std::size_t>(pop_lsb(wn)); early_score += EARLY_KNIGHT_EVAL + EARLY_KNIGHT_SQ[sq]; late_score += LATE_KNIGHT_EVAL + LATE_KNIGHT_SQ[sq]; }
    while (wb != 0) { auto sq = static_cast<std::size_t>(pop_lsb(wb)); early_score += EARLY_BISHOP_EVAL + EARLY_BISHOP_SQ[sq]; late_score += LATE_BISHOP_EVAL + LATE_BISHOP_SQ[sq]; }
    while (wr != 0) { auto sq = static_cast<std::size_t>(pop_lsb(wr)); early_score += EARLY_ROOK_EVAL   + EARLY_ROOK_SQ[sq];   late_score += LATE_ROOK_EVAL   + LATE_ROOK_SQ[sq]; }
    while (wq != 0) { auto sq = static_cast<std::size_t>(pop_lsb(wq)); early_score += EARLY_QUEEN_EVAL  + EARLY_QUEEN_SQ[sq];  late_score += LATE_QUEEN_EVAL  + LATE_QUEEN_SQ[sq]; }
    early_score += EARLY_KING_SQ[static_cast<std::size_t>(lsb(wk))];
    late_score += LATE_KING_SQ[static_cast<std::size_t>(lsb(wk))];

    auto bp = b.black_pieces & b.pawns;
    auto bn = b.black_pieces & b.knights;
    auto bb = b.black_pieces & b.bishops;
    auto br = b.black_pieces & b.rooks;
    auto bq = b.black_pieces & b.queens;
    auto bk = b.black_pieces & b.kings;
    while (bp != 0) { auto sq = static_cast<std::size_t>(pop_lsb(bp)); early_score -= EARLY_PAWN_EVAL   + EARLY_PAWN_SQ[sq^56];   late_score -= LATE_PAWN_EVAL   + LATE_PAWN_SQ[sq^56]; }
    while (bn != 0) { auto sq = static_cast<std::size_t>(pop_lsb(bn)); early_score -= EARLY_KNIGHT_EVAL + EARLY_KNIGHT_SQ[sq^56]; late_score -= LATE_KNIGHT_EVAL + LATE_KNIGHT_SQ[sq^56]; }
    while (bb != 0) { auto sq = static_cast<std::size_t>(pop_lsb(bb)); early_score -= EARLY_BISHOP_EVAL + EARLY_BISHOP_SQ[sq^56]; late_score -= LATE_BISHOP_EVAL + LATE_BISHOP_SQ[sq^56]; }
    while (br != 0) { auto sq = static_cast<std::size_t>(pop_lsb(br)); early_score -= EARLY_ROOK_EVAL   + EARLY_ROOK_SQ[sq^56];   late_score -= LATE_ROOK_EVAL   + LATE_ROOK_SQ[sq^56]; }
    while (bq != 0) { auto sq = static_cast<std::size_t>(pop_lsb(bq)); early_score -= EARLY_QUEEN_EVAL  + EARLY_QUEEN_SQ[sq^56];  late_score -= LATE_QUEEN_EVAL  + LATE_QUEEN_SQ[sq^56]; }
    early_score -= EARLY_KING_SQ[static_cast<std::size_t>(lsb(bk))^56];
    late_score -= LATE_KING_SQ[static_cast<std::size_t>(lsb(bk))^56];

    int tapered_score = (early_score * material_score + late_score * (max_material - material_score)) / max_material;

    return b.white_to_move ? tapered_score : -tapered_score;
}

} // namespace azalea
