#pragma once

#include "eval.hpp"

#include <array>

namespace azalea {

inline constexpr Score TEMPO_EVAL = {49, 0};

inline constexpr std::array<Score, 5> MATERIAL = {{
    // early late
    { 144,  101}, // pawn
    { 589,  272}, // knight
    { 610,  303}, // bishop
    { 798,  574}, // rook
    {1694, 1075}  // queen
}};

inline constexpr Score BISHOP_PAIR_EVAL = {39, 62};

inline constexpr std::array<Score, 8> PASSED_PAWN_EVAL = {{
    // early late
    {  0,   0}, // rank 1
    {  8, -26}, // rank 2
    { -3, -13}, // rank 3
    {-12,  22}, // rank 4
    {  8,  59}, // rank 5
    {  2, 141}, // rank 6
    { 70, 199}, // rank 7
    {  0,   0}, // rank 8
}};

inline constexpr std::array<Score, 8> PASSED_PAWN_KING_DISTANCE_EVAL = {{
 {0,  0}, // distance 0
 {0, 75}, // distance 1
 {0, 53}, // distance 2
 {0, 26}, // distance 2
 {0, 17}, // distance 4
 {0, 10}, // distance 5
 {0,  9}, // distance 6
 {0, -1}, // distance 7
}};

inline constexpr Score ISOLATED_PAWN_EVAL = {-27, -5};
inline constexpr Score DOUBLED_PAWN_EVAL = {-4, -10};

inline constexpr Score ROOK_HALF_OPEN_EVAL = {33, 13};
inline constexpr Score ROOK_OPEN_EVAL = {76, -5};

inline constexpr std::array<Score, 64> PAWN_SQ = {{
    {  0,  0},  {  0,  0},  {  0,  0},  {  0,   0},  {  0,   0},  {  0,   0},  {  0,   0},  {  0,   0},
    {-45, 25},  {-32, 15},  {-31, 19},  {-39,  10},  {-18,   5},  { 38,  -4},  { 40, -14},  {-18, -11},
    {-33, 12},  {-37, 11},  { -4, -5},  {-18,   0},  {  7,  -4},  {  1,  -6},  { 30, -15},  { -3, -15},
    {-34, 20},  {-31, 16},  {-12, -7},  { 20, -21},  { 12, -18},  { 14, -17},  { -6,  -8},  {-25,  -8},
    { -4, 32},  { -8, 20},  {  3,  1},  { 23, -22},  { 39, -24},  { 30, -15},  { -1,   3},  {-16,   3},
    { -2, 51},  { -7, 35},  { 40, 14},  { 24, -17},  { 61, -30},  {129, -30},  { 22,   5},  {  6,  19},
    { 30, 45},  { -5, 45},  {-23, 35},  { 17,  -7},  { 58, -23},  { 54, -13},  {-39,  45},  {-34,  33},
    {  0,  0},  {  0,  0},  {  0,  0},  {  0,   0},  {  0,   0},  {  0,   0},  {  0,   0},  {  0,   0},
}};

inline constexpr std::array<Score, 64> KNIGHT_SQ = {{
    { -50, -46}, { -35,  -35}, { -56,    6}, { -31,    8}, { -27,   -3}, { -17,    7}, { -41,  -31}, { -58,  -26},
    { -27, -20}, { -30,   -4}, {   0,    3}, {   3,   20}, {   6,   11}, {  20,    8}, {   4,   -7}, { -11,  -18},
    { -24,  -7}, {   0,   16}, {  22,   13}, {  23,   38}, {  32,   39}, {  15,   22}, {  36,    1}, { -27,  -13},
    {  -6,  -3}, {  12,   18}, {  32,   39}, {  27,   42}, {  43,   46}, {  39,   24}, {  22,    8}, { -25,    9},
    {  25,   3}, {  29,   26}, {  32,   36}, {  79,   48}, {  36,   49}, {  72,   36}, {  13,   29}, {  38,  -12},
    {   3, -18}, {  28,    0}, {  53,   28}, {  94,   23}, { 136,  -10}, { 151,    4}, {  85,  -10}, {   7,  -14},
    { -78, -13}, { -16,   -7}, { 102,  -19}, {  31,    7}, { 111,  -26}, {  96,  -21}, {   0,  -19}, {   2,  -42},
    {-258, -29}, { -32,  -43}, { -41,   -9}, { -61,   -3}, {  21,  -17}, {-124,  -14}, { -36,  -29}, {-142,  -77},
}};

inline constexpr std::array<Score, 64> BISHOP_SQ = {{
    { -4,  -28},  {-20,   -7},  {-23,  -27},  {-21,    0},  {-29,   -1},  {-43,   -4},  { -5,  -18},  {-18,  -17},
    { -8,   -1},  { 25,  -23},  { 10,   -5},  { -5,   10},  {  0,   15},  { 28,   -1},  { 41,  -11},  {  4,  -27},
    { 13,  -16},  { 26,    4},  { 20,   11},  {  6,   26},  { 11,   27},  { 20,   13},  { 16,  -11},  { 16,  -16},
    {-11,   -2},  {  3,    5},  {  9,   25},  { 33,   17},  { 45,   14},  {  5,   24},  { 11,    0},  {-30,   -9},
    {-32,   16},  { -4,   20},  { 22,   17},  { 39,   26},  { 38,   20},  { 10,   14},  {  3,    6},  { -4,   -8},
    {-15,    8},  {  8,    6},  { 75,   -1},  { 24,   18},  { 76,    2},  { 60,   18},  { 52,    3},  { 45,  -15},
    {-52,   -2},  { 15,    0},  {-17,    7},  {  4,   -9},  { 18,    3},  { 56,   -8},  { -5,    7},  { 46,  -43},
    {-37,   -6},  {-44,  -17},  {-25,  -19},  {-74,   -2},  {-80,    3},  {-50,   -3},  {-13,   -4},  {-55,    4},
}};

inline constexpr std::array<Score, 64> ROOK_SQ = {{
    {-33,   7}, { -33,   7}, { -26,   7}, { -24,  13}, {-12,  -5}, {-11,   0}, { -41,    2}, { -17,  -38},
    {-59,   3}, { -47,   4}, { -56,  16}, { -42,   5}, {-35,   1}, {-18,  -4}, {   0,  -13}, { -76,    4},
    {-55,  -2}, { -47,   6}, { -42,   3}, { -30,   1}, {-31,   3}, {-25,  -4}, {  26,  -26}, { -12,  -23},
    {-49,  11}, { -58,  15}, { -63,  19}, { -44,  17}, {-35,  13}, {-24,   2}, {   4,  -11}, { -22,  -13},
    {-46,  18}, { -32,  10}, { -19,  16}, {   1,   4}, {-17,   4}, {  7,   0}, {  23,   -6}, {  30,  -12},
    {-14,  14}, { -13,  13}, { -25,  13}, {   1,   4}, { 27,  -5}, { 52,  -8}, {  54,   -9}, {  56,  -19},
    {-18,  17}, { -36,  35}, {   8,  18}, {  46,   3}, {  7,   7}, { 73,  -3}, {  80,   -7}, {  60,  -12},
    {  8,  22}, {  18,  15}, {  35,  12}, {  38,   5}, { 66,  -2}, { 66,  -5}, {  44,   -2}, {   1,   11},
}};

inline constexpr std::array<Score, 64> QUEEN_SQ = {{
    {-14, -30}, { -30, -32}, { -17, -32}, {  -2, -82}, {-21, -16}, {-37, -41}, {-58, -31}, {-28, -42},
    {-31, -17}, {  -6, -12}, {   0, -32}, {  -5,  -7}, {  3, -17}, { 14, -48}, { 19, -60}, { -8, -35},
    {-24, -47}, {  -4, -45}, { -16,  16}, {  -9,  -2}, {-14,  18}, { -6,  13}, {  7,  14}, { -3, -26},
    {-32,  -9}, { -12,   9}, { -17,  14}, { -20,  52}, {  1,  32}, { -6,  15}, {  5,  10}, {-10,  -6},
    {-41, -19}, { -20,  19}, {  -7,   9}, { -10,  44}, { -6,  60}, {  7,  60}, {-18,  42}, { 17,   5},
    {-27, -36}, { -27,  -1}, {  16,  19}, {  36,  12}, { 60,  51}, { 82,  15}, {128, -19}, { 75, -21},
    {-40,  -6}, { -48,   4}, { -46,  32}, { -54,  62}, {-39,  51}, {103,  52}, { 24,  51}, {126, -60},
    {-59,   9}, {   2,   8}, {  38,   8}, {  48,  24}, { 54,  14}, { 53,  17}, { -3,   0}, { -6,  22},
}};

inline constexpr std::array<Score, 64> KING_SQ = {{
     {-50, -11}, { 44, -23}, {  4,  -3}, {-83, 11}, { -4, -19}, {-45,  7}, { 59,-32}, { 53, -55},
     { 36, -21}, {-13,   5}, {-23,  20}, {-99, 41}, {-73,  36}, {-31, 29}, { 36,  6}, { 31,  -7},
     {  5, -29}, { 19,  -8}, {-57,  20}, {-78, 31}, {-69,  33}, {-37, 26}, { 14,  6}, {-23,   0},
     {  8, -39}, { 18, -19}, { -3,   5}, {-57, 20}, {-43,  24}, {-52, 23}, {-10,  3}, {-62,  -2},
     { -3, -39}, {  9, -12}, { 29,  -2}, { -1, 11}, {-13,  11}, { -2, 17}, { 14,  9}, {-20,  -1},
     { 10, -23}, { 84,  -6}, { 19,   2}, { 38, -3}, {  6,   9}, { 42, 21}, { 50, 25}, {  7,   9},
     {  7, -22}, { 46,   3}, { 23,   1}, { 26, -4}, { 19,   2}, { 38, 19}, { -3, 27}, {-11,   8},
     { 13, -46}, { 16, -17}, { 17, -16}, {  8, -8}, { 24,  -8}, { 20,  1}, { 11, -2}, {  4, -27},
}};

inline constexpr std::array<Score, 9> KNIGHT_MOBILITY_EVAL = {{
    {0, 0}, // 0 squares
    {0, 0}, // 1 squares
    {0, 0}, // 2 squares
    {0, 0}, // 3 squares
    {0, 0}, // 4 squares
    {0, 0}, // 5 squares
    {0, 0}, // 6 squares
    {0, 0}, // 7 squares
    {0, 0}, // 8 squares
}};

inline constexpr std::array<Score, 14> BISHOP_MOBILITY_EVAL = {{
    {0, 0}, //  0 squares
    {0, 0}, //  1 squares
    {0, 0}, //  2 squares
    {0, 0}, //  3 squares
    {0, 0}, //  4 squares
    {0, 0}, //  5 squares
    {0, 0}, //  6 squares
    {0, 0}, //  7 squares
    {0, 0}, //  8 squares
    {0, 0}, //  9 squares
    {0, 0}, // 10 squares
    {0, 0}, // 11 squares
    {0, 0}, // 12 squares
    {0, 0}, // 13 squares
}};

inline constexpr std::array<Score, 15> ROOK_MOBILITY_EVAL = {{
    {0, 0}, //  0 squares
    {0, 0}, //  1 squares
    {0, 0}, //  2 squares
    {0, 0}, //  3 squares
    {0, 0}, //  4 squares
    {0, 0}, //  5 squares
    {0, 0}, //  6 squares
    {0, 0}, //  7 squares
    {0, 0}, //  8 squares
    {0, 0}, //  9 squares
    {0, 0}, // 10 squares
    {0, 0}, // 11 squares
    {0, 0}, // 12 squares
    {0, 0}, // 13 squares
    {0, 0}, // 14 squares
}};

inline constexpr std::array<Score, 28> QUEEN_MOBILITY_EVAL = {{
    {0, 0}, //  0 squares
    {0, 0}, //  1 squares
    {0, 0}, //  2 squares
    {0, 0}, //  3 squares
    {0, 0}, //  4 squares
    {0, 0}, //  5 squares
    {0, 0}, //  6 squares
    {0, 0}, //  7 squares
    {0, 0}, //  8 squares
    {0, 0}, //  9 squares
    {0, 0}, // 10 squares
    {0, 0}, // 11 squares
    {0, 0}, // 12 squares
    {0, 0}, // 13 squares
    {0, 0}, // 14 squares
    {0, 0}, // 15 squares
    {0, 0}, // 16 squares
    {0, 0}, // 17 squares
    {0, 0}, // 18 squares
    {0, 0}, // 19 squares
    {0, 0}, // 20 squares
    {0, 0}, // 21 squares
    {0, 0}, // 22 squares
    {0, 0}, // 23 squares
    {0, 0}, // 24 squares
    {0, 0}, // 25 squares
    {0, 0}, // 26 squares
    {0, 0}, // 27 squares
}};

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
