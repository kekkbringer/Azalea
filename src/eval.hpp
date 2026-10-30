#pragma once

#include "board.hpp"
#include "engine.hpp"
#include "util.hpp"

#include "eval_trace.hpp" // for the tuner only

#include <array>
#include <cstdlib> // std::abs

namespace azalea {

constexpr int SCORE_INF  = 1048576;
constexpr int SCORE_MATE = 1048575;
constexpr int SCORE_NONE = SCORE_INF + 1;
constexpr int MATE_BOUND = SCORE_MATE - MAX_PLY;

constexpr bool is_mate_score(int s) { return std::abs(s) >= MATE_BOUND; }


// helper functions
constexpr Bitboard gen_passed_pawn_mask(const int sq) {
    Bitboard mask = 0ULL;
    const int file = sq % 8;
    const int rank = sq / 8;
    const Bitboard middle_mask = (1ULL << sq);
    const Bitboard left_mask  = file > 0 ? middle_mask >> 1 : 0ULL;
    const Bitboard right_mask = file < 7 ? middle_mask << 1 : 0ULL;
    Bitboard combined_mask = middle_mask | left_mask | right_mask;
    for (int i=rank; i<8; ++i) {
        combined_mask <<= 8;
        mask |= combined_mask;
    }
    return mask;
}

constexpr std::array<Bitboard, 64> gen_passed_pawn_table() {
    std::array<Bitboard, 64> table;
    for (int sq=0; sq<64; ++sq)
        table[static_cast<std::size_t>(sq)] = gen_passed_pawn_mask(sq);
    return table;
}

constexpr std::array<std::array<int, 64>, 64> gen_distance_table() {
    std::array<std::array<int, 64>, 64> table;
    for (int i=0; i<64; ++i) {
        const int i_file = i % 8;
        const int i_rank = i / 8;
        for (int j=0; j<64; ++j) {
            const int j_file = j % 8;
            const int j_rank = j / 8;
            const int file_diff = std::abs(i_file - j_file);
            const int rank_diff = std::abs(i_rank - j_rank);
            if (file_diff <= rank_diff)
                table[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = rank_diff;
            else
                table[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = file_diff;
        }
    }
    return table;
}

constexpr auto PASSED_PAWN_MASK_TABLE = gen_passed_pawn_table();
constexpr auto DISTANCE_TABLE = gen_distance_table();


constexpr int PAWN_MATERIAL   =  0;
constexpr int KNIGHT_MATERIAL =  1;
constexpr int BISHOP_MATERIAL =  1;
constexpr int ROOK_MATERIAL   =  2;
constexpr int QUEEN_MATERIAL  =  3;


int eval(const Board&);
void trace_eval(const Board&, EvalTrace&);

} // namespace azalea
