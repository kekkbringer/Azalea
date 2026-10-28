#pragma once

#include "board.hpp"
#include "engine.hpp"

#include "eval_trace.hpp" // for the tuner only

#include <array>
#include <cstdlib> // std::abs

namespace azalea {

constexpr int SCORE_INF  = 1048576;
constexpr int SCORE_MATE = 1048575;
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

constexpr int TEMPO_EVAL = 47;

constexpr int EARLY_PAWN_EVAL   = 136;
constexpr int EARLY_KNIGHT_EVAL = 559;
constexpr int EARLY_BISHOP_EVAL = 578;
constexpr int EARLY_ROOK_EVAL   = 799;
constexpr int EARLY_QUEEN_EVAL  = 1638;

constexpr int LATE_PAWN_EVAL   = 97;
constexpr int LATE_KNIGHT_EVAL = 260;
constexpr int LATE_BISHOP_EVAL = 290;
constexpr int LATE_ROOK_EVAL   = 549;
constexpr int LATE_QUEEN_EVAL  = 1009;

constexpr int EARLY_BISHOP_PAIR_EVAL = 36;
constexpr int LATE_BISHOP_PAIR_EVAL = 57;

constexpr int EARLY_PASSED_PAWN[8] = {0, -10, -19, -25, -3, -10, 81, 0};
constexpr int LATE_PASSED_PAWN[8] = {0, -30, -16, 17, 53, 132, 207, 0};
constexpr int PASSED_PAWN_KING_DISTANCE_EVAL[8] = {0, 87, 67, 40, 31, 24, 24, 15};

constexpr int EARLY_ISOLATED_PAWN = -10;
constexpr int LATE_ISOLATED_PAWN = -20;

constexpr int EARLY_DOUBLED_PAWN = -15;
constexpr int LATE_DOUBLED_PAWN = -25;

constexpr int EARLY_PAWN_SQ[64] = {
       0,    0,    0,    0,    0,    0,    0,    0,
     -59,   -4,  -27,  -35,  -14,   25,   61,  -25,
     -49,  -11,   -5,  -16,    9,  -16,   49,  -11,
     -50,   -7,  -12,   18,   13,   -1,   14,  -33,
     -23,    7,   -3,   17,   34,   13,   15,  -28,
     -25,    5,   36,   23,   58,  118,   40,   -5,
     -13,  -13,  -47,   -6,   53,   58,  -52,  -83,
       0,    0,    0,    0,    0,    0,    0,    0,
};

constexpr int LATE_PAWN_SQ[64] = {
       0,    0,    0,    0,    0,    0,    0,    0,
      29,   16,   15,   13,    4,   -1,  -10,   -7,
      16,   10,   -9,    0,   -5,   -4,  -12,  -11,
      23,   15,   -8,  -20,  -17,  -15,   -6,   -6,
      33,   18,   -1,  -22,  -25,  -14,    3,    5,
      50,   30,   10,  -18,  -31,  -31,    2,   18,
      20,   18,   12,  -31,  -52,  -42,   23,   13,
       0,    0,    0,    0,    0,    0,    0,    0,
};

constexpr int EARLY_KNIGHT_SQ[64] = {
     -62,  -33,  -62,  -32,  -30,  -24,  -40,  -80,
     -28,  -34,   -3,    2,    3,   18,    4,  -13,
     -22,   -3,   19,   18,   28,   11,   31,  -26,
      -8,   11,   28,   25,   40,   35,   17,  -28,
      29,   26,   31,   76,   33,   72,    8,   35,
      18,   36,   55,   99,  151,  177,  103,    7,
     -67,   -8,  115,   37,  136,  120,   12,   11,
    -100,  -60,  -40,  -40,  -40,  -40,  -60, -100,
};

constexpr int LATE_KNIGHT_SQ[64] = {
     -52,  -33,    9,   10,   -2,    8,  -29,  -23,
     -22,   -4,    2,   18,   10,    7,  -10,  -20,
      -7,   17,   12,   37,   38,   20,    2,  -13,
      -1,   17,   37,   40,   45,   23,    8,   10,
       1,   27,   35,   45,   48,   33,   30,  -12,
     -23,   -3,   26,   19,  -19,   -6,  -16,  -14,
     -11,   -8,  -25,    4,  -38,  -28,  -22,  -46,
      13,  -38,   -4,    6,  -28,    3,   -6,  -66,
};

constexpr int EARLY_BISHOP_SQ[64] = {
      -1,  -28,  -23,  -26,  -30,  -45,   -8,  -23,
     -13,   23,    2,   -7,   -6,   29,   35,    1,
      14,   18,   16,    0,    7,   14,   11,   11,
     -14,    5,    3,   33,   41,    3,    6,  -31,
     -36,   -9,   26,   38,   36,    7,   -1,  -10,
     -13,   14,   77,   28,   82,   72,   57,   48,
     -38,   23,  -12,   13,   29,   76,   10,   46,
     -50,  -40,  -30,  -30,  -30,  -30,  -40,  -50,
};

constexpr int LATE_BISHOP_SQ[64] = {
     -32,   -6,  -30,    0,   -2,   -6,  -19,  -17,
      -1,  -23,   -6,    7,   13,   -4,  -11,  -27,
     -18,    5,    9,   23,   25,   12,  -12,  -16,
      -2,    3,   25,   14,   14,   22,    0,   -8,
      17,   20,   14,   24,   19,   14,    6,   -7,
       7,    3,   -3,   16,   -3,   13,    2,  -17,
      -4,   -1,    4,  -13,   -1,  -14,    5,  -46,
      -1,  -12,  -18,    5,   18,    3,   -1,   13,
};

constexpr int EARLY_ROOK_SQ[64] = {
     -60,  -50,  -26,  -25,  -20,  -43,  -62,  -47,
     -86,  -53,  -56,  -43,  -41,  -40,  -15, -103,
     -77,  -54,  -40,  -30,  -35,  -50,   16,  -32,
     -65,  -65,  -60,  -35,  -30,  -38,   -9,  -37,
     -53,  -31,   -2,   19,   -3,    0,   21,   30,
     -15,    4,    0,   30,   59,   65,   85,   75,
       4,   -7,   39,   84,   32,   90,   90,   85,
      37,   45,   78,   84,   90,   90,   90,   10,
};

constexpr int LATE_ROOK_SQ[64] = {
       7,   13,    8,   21,    2,    2,    9,  -35,
      12,    7,   17,    9,    4,    3,   -9,   12,
       4,    8,    3,    3,    6,    2,  -27,  -19,
      15,   16,   19,   17,   13,    4,  -12,  -11,
      18,    9,   11,    0,    2,    1,  -10,  -17,
      13,    7,    7,   -3,  -14,  -14,  -21,  -28,
       8,   24,    9,   -9,    0,  -18,  -25,  -25,
      11,    5,   -1,   -8,  -26,  -33,  -18,    4,
};

constexpr int EARLY_QUEEN_SQ[64] = {
     -22,  -38,  -24,   -9,  -31,  -46,  -88,  -39,
     -40,  -14,   -9,  -13,   -7,    9,   16,   -2,
     -29,  -13,  -25,  -19,  -22,  -16,   -2,  -11,
     -39,  -20,  -27,  -25,   -7,  -17,   -7,  -21,
     -43,  -29,  -11,  -17,  -15,   -7,  -29,    8,
     -28,  -30,   13,   35,   59,  101,  148,   69,
     -30,  -49,  -45,  -74,  -54,  150,   28,  171,
     -72,   -1,   75,  112,  133,  142,  -12,  -25,
};

constexpr int LATE_QUEEN_SQ[64] = {
     -28,  -35,  -35,  -83,  -15,  -39,  -22,  -55,
     -12,   -8,  -27,   -4,  -15,  -53,  -70,  -54,
     -45,  -41,   21,   -1,   21,   19,   15,  -28,
      -2,   14,   19,   55,   34,   20,   13,   -3,
     -11,   26,   13,   48,   69,   73,   51,   11,
     -32,    6,   22,   12,   52,    5,  -38,  -23,
      -3,    9,   37,   82,   67,   30,   56,    0,
      19,   12,  -11,   -9,  -27,  -24,    5,   35,
};

constexpr int EARLY_KING_SQ[64] = {
     -92,    9,  -28, -113,  -34,  -75,   27,   23,
      25,  -46,  -57, -128, -104,  -65,    7,    3,
     -18,   -3,  -97, -113, -104,  -69,  -15,  -52,
      14,   15,  -27, -107,  -80,  -90,  -38, -108,
     -16,    3,   32,  -23,  -41,  -21,    4,  -52,
      32,  188,   24,   72,   -1,   79,   98,    2,
      54,  146,   60,   69,   44,  107,  -16,  -61,
     141,   91,  113,   59,  133,  117,   64,   44,
};

constexpr int LATE_KING_SQ[64] = {
      -2,  -17,    3,   16,  -14,   11,  -26,  -49,
     -21,    9,   25,   44,   40,   34,   10,   -3,
     -26,   -5,   26,   36,   38,   30,    9,    5,
     -40,  -18,    9,   27,   29,   29,    7,    6,
     -38,  -11,   -3,   13,   14,   19,    9,    4,
     -26,  -23,    1,   -9,    9,   14,   16,    8,
     -31,  -13,   -5,  -11,   -2,    8,   29,   16,
     -70,  -31,  -30,  -15,  -23,  -13,  -10,  -38,
};

int eval(const Board&);
void trace_eval(const Board&, EvalTrace&);

} // namespace azalea
