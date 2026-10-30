#pragma once

#include "eval.hpp"
#include "weights_data.hpp"

#include <array>

namespace azalea {

inline constexpr std::array<Score, N_TRACE> WEIGHTS = [] {
    std::array<Score, N_TRACE> w{};

    auto place = [&w](int start, const auto& table) {
        for (std::size_t i=0; i<table.size(); ++i)
            w[static_cast<std::size_t>(start) + i] = table[i];
    };

    w[TEMPO]          = TEMPO_EVAL;
    w[BISHOP_PAIR]    = BISHOP_PAIR_EVAL;
    w[ROOK_HALF_OPEN] = ROOK_HALF_OPEN_EVAL;
    w[ROOK_OPEN]      = ROOK_OPEN_EVAL;
    w[ISOLATED_PAWN]  = ISOLATED_PAWN_EVAL;
    w[DOUBLED_PAWN]   = DOUBLED_PAWN_EVAL;

    place(PAWN_EVAL,   MATERIAL);
    place(PASSED_PAWN_START, PASSED_PAWN_EVAL);
    place(PASSED_PAWN_KING_DISTANCE, PASSED_PAWN_KING_DISTANCE_EVAL);
    place(PST_START + 0 * 64, PAWN_SQ);
    place(PST_START + 1 * 64, KNIGHT_SQ);
    place(PST_START + 2 * 64, BISHOP_SQ);
    place(PST_START + 3 * 64, ROOK_SQ);
    place(PST_START + 4 * 64, QUEEN_SQ);
    place(PST_START + 5 * 64, KING_SQ);
    place(KNIGHT_MOBILITY, KNIGHT_MOBILITY_EVAL);
    place(BISHOP_MOBILITY, BISHOP_MOBILITY_EVAL);
    place(ROOK_MOBILITY,   ROOK_MOBILITY_EVAL);
    place(QUEEN_MOBILITY,  QUEEN_MOBILITY_EVAL);

    return w;
}();

} // namespace azalea
