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

constexpr Bitboard FILES[8] = {
    0x0101010101010101,
    0x0202020202020202,
    0x0404040404040404,
    0x0808080808080808,
    0x1010101010101010,
    0x2020202020202020,
    0x4040404040404040,
    0x8080808080808080,
};

constexpr Bitboard ADJACENT_FILES[8] = {
    FILES[1],
    FILES[0] | FILES[2],
    FILES[1] | FILES[3],
    FILES[2] | FILES[4],
    FILES[3] | FILES[5],
    FILES[4] | FILES[6],
    FILES[5] | FILES[7],
    FILES[6],
};


constexpr int PAWN_MATERIAL   =  0;
constexpr int KNIGHT_MATERIAL =  1;
constexpr int BISHOP_MATERIAL =  1;
constexpr int ROOK_MATERIAL   =  2;
constexpr int QUEEN_MATERIAL  =  3;

constexpr int TEMPO_EVAL = 49;

constexpr int EARLY_PAWN_EVAL = 144;
constexpr int EARLY_KNIGHT_EVAL = 589;
constexpr int EARLY_BISHOP_EVAL = 610;
constexpr int EARLY_ROOK_EVAL = 798;
constexpr int EARLY_QUEEN_EVAL = 1694;

constexpr int LATE_PAWN_EVAL = 101;
constexpr int LATE_KNIGHT_EVAL = 272;
constexpr int LATE_BISHOP_EVAL = 303;
constexpr int LATE_ROOK_EVAL = 574;
constexpr int LATE_QUEEN_EVAL = 1075;

constexpr int EARLY_BISHOP_PAIR_EVAL = 39;
constexpr int LATE_BISHOP_PAIR_EVAL = 62;

constexpr int EARLY_PASSED_PAWN[8] = {0, 8, -3, -12, 8, 2, 70, 0};
constexpr int LATE_PASSED_PAWN[8] = {0, -26, -13, 22, 59, 141, 199, 0};
constexpr int PASSED_PAWN_KING_DISTANCE_EVAL[8] = {0, 75, 53, 26, 17, 10, 9, -1};

constexpr int EARLY_ISOLATED_PAWN = -27;
constexpr int LATE_ISOLATED_PAWN = -5;

constexpr int EARLY_DOUBLED_PAWN = -4;
constexpr int LATE_DOUBLED_PAWN = -10;

constexpr int EARLY_ROOK_HALF_OPEN = 33;
constexpr int LATE_ROOK_HALF_OPEN = 13;

constexpr int EARLY_ROOK_OPEN = 76;
constexpr int LATE_ROOK_OPEN = -5;

constexpr int EARLY_PAWN_SQ[64] = {
       0,    0,    0,    0,    0,    0,    0,    0,
     -45,  -32,  -31,  -39,  -18,   38,   40,  -18,
     -33,  -37,   -4,  -18,    7,    1,   30,   -3,
     -34,  -31,  -12,   20,   12,   14,   -6,  -25,
      -4,   -8,    3,   23,   39,   30,   -1,  -16,
      -2,   -7,   40,   24,   61,  129,   22,    6,
      30,   -5,  -23,   17,   58,   54,  -39,  -34,
       0,    0,    0,    0,    0,    0,    0,    0,
};

constexpr int LATE_PAWN_SQ[64] = {
       0,    0,    0,    0,    0,    0,    0,    0,
      25,   15,   19,   10,    5,   -4,  -14,  -11,
      12,   11,   -5,    0,   -4,   -6,  -15,  -15,
      20,   16,   -7,  -21,  -18,  -17,   -8,   -8,
      32,   20,    1,  -22,  -24,  -15,    3,    3,
      51,   35,   14,  -17,  -30,  -30,    5,   19,
      45,   45,   35,   -7,  -23,  -13,   45,   33,
       0,    0,    0,    0,    0,    0,    0,    0,
};

constexpr int EARLY_KNIGHT_SQ[64] = {
     -50,  -35,  -56,  -31,  -27,  -17,  -41,  -58,
     -27,  -30,    0,    3,    6,   20,    4,  -11,
     -24,    0,   22,   23,   32,   15,   36,  -27,
      -6,   12,   32,   27,   43,   39,   22,  -25,
      25,   29,   32,   79,   36,   72,   13,   38,
       3,   28,   53,   94,  136,  151,   85,    7,
     -78,  -16,  102,   31,  111,   96,    0,    2,
    -258,  -32,  -41,  -61,   21, -124,  -36, -142,
};

constexpr int LATE_KNIGHT_SQ[64] = {
     -46,  -35,    6,    8,   -3,    7,  -31,  -26,
     -20,   -4,    3,   20,   11,    8,   -7,  -18,
      -7,   16,   13,   38,   39,   22,    1,  -13,
      -3,   18,   39,   42,   46,   24,    8,    9,
       3,   26,   36,   48,   49,   36,   29,  -12,
     -18,    0,   28,   23,  -10,    4,  -10,  -14,
     -13,   -7,  -19,    7,  -26,  -21,  -19,  -42,
     -29,  -43,   -9,   -3,  -17,  -14,  -29,  -77,
};

constexpr int EARLY_BISHOP_SQ[64] = {
      -4,  -20,  -23,  -21,  -29,  -43,   -5,  -18,
      -8,   25,   10,   -5,    0,   28,   41,    4,
      13,   26,   20,    6,   11,   20,   16,   16,
     -11,    3,    9,   33,   45,    5,   11,  -30,
     -32,   -4,   22,   39,   38,   10,    3,   -4,
     -15,    8,   75,   24,   76,   60,   52,   45,
     -52,   15,  -17,    4,   18,   56,   -5,   46,
     -37,  -44,  -25,  -74,  -80,  -50,  -13,  -55,
};

constexpr int LATE_BISHOP_SQ[64] = {
     -28,   -7,  -27,    0,   -1,   -4,  -18,  -17,
      -1,  -23,   -5,   10,   15,   -1,  -11,  -27,
     -16,    4,   11,   26,   27,   13,  -11,  -16,
      -2,    5,   25,   17,   14,   24,    0,   -9,
      16,   20,   17,   26,   20,   14,    6,   -8,
       8,    6,   -1,   18,    2,   18,    3,  -15,
      -2,    0,    7,   -9,    3,   -8,    7,  -43,
      -6,  -17,  -19,   -2,    3,   -3,   -4,    4,
};

constexpr int EARLY_ROOK_SQ[64] = {
     -33,  -33,  -26,  -24,  -12,  -11,  -41,  -17,
     -59,  -47,  -56,  -42,  -35,  -18,    0,  -76,
     -55,  -47,  -42,  -30,  -31,  -25,   26,  -12,
     -49,  -58,  -63,  -44,  -35,  -24,    4,  -22,
     -46,  -32,  -19,    1,  -17,    7,   23,   30,
     -14,  -13,  -25,    1,   27,   52,   54,   56,
     -18,  -36,    8,   46,    7,   73,   80,   60,
       8,   18,   35,   38,   66,   66,   44,    1,
};

constexpr int LATE_ROOK_SQ[64] = {
       7,    7,    7,   13,   -5,    0,    2,  -38,
       3,    4,   16,    5,    1,   -4,  -13,    4,
      -2,    6,    3,    1,    3,   -4,  -26,  -23,
      11,   15,   19,   17,   13,    2,  -11,  -13,
      18,   10,   16,    4,    4,    0,   -6,  -12,
      14,   13,   13,    4,   -5,   -8,   -9,  -19,
      17,   35,   18,    3,    7,   -3,   -7,  -12,
      22,   15,   12,    5,   -2,   -5,   -2,   11,
};

constexpr int EARLY_QUEEN_SQ[64] = {
     -14,  -30,  -17,   -2,  -21,  -37,  -58,  -28,
     -31,   -6,    0,   -5,    3,   14,   19,   -8,
     -24,   -4,  -16,   -9,  -14,   -6,    7,   -3,
     -32,  -12,  -17,  -20,    1,   -6,    5,  -10,
     -41,  -20,   -7,  -10,   -6,    7,  -18,   17,
     -27,  -27,   16,   36,   60,   82,  128,   75,
     -40,  -48,  -46,  -54,  -39,  103,   24,  126,
     -59,    2,   38,   48,   54,   53,   -3,   -6,
};

constexpr int LATE_QUEEN_SQ[64] = {
     -30,  -32,  -32,  -82,  -16,  -41,  -31,  -42,
     -17,  -12,  -32,   -7,  -17,  -48,  -60,  -35,
     -47,  -45,   16,   -2,   18,   13,   14,  -26,
      -9,    9,   14,   52,   32,   15,   10,   -6,
     -19,   19,    9,   44,   60,   60,   42,    5,
     -36,   -1,   19,   12,   51,   15,  -19,  -21,
      -6,    4,   32,   62,   51,   52,   51,  -60,
       9,    8,    8,   24,   14,   17,    0,   22,
};

constexpr int EARLY_KING_SQ[64] = {
     -50,   44,    4,  -83,   -4,  -45,   59,   53,
      36,  -13,  -23,  -99,  -73,  -31,   36,   31,
       5,   19,  -57,  -78,  -69,  -37,   14,  -23,
       8,   18,   -3,  -57,  -43,  -52,  -10,  -62,
      -3,    9,   29,   -1,  -13,   -2,   14,  -20,
      10,   84,   19,   38,    6,   42,   50,    7,
       7,   46,   23,   26,   19,   38,   -3,  -11,
      13,   16,   17,    8,   24,   20,   11,    4,
};

constexpr int LATE_KING_SQ[64] = {
     -11,  -23,   -3,   11,  -19,    7,  -32,  -55,
     -21,    5,   20,   41,   36,   29,    6,   -7,
     -29,   -8,   20,   31,   33,   26,    6,    0,
     -39,  -19,    5,   20,   24,   23,    3,   -2,
     -39,  -12,   -2,   11,   11,   17,    9,   -1,
     -23,   -6,    2,   -3,    9,   21,   25,    9,
     -22,    3,    1,   -4,    2,   19,   27,    8,
     -46,  -17,  -16,   -8,   -8,    1,   -2,  -27,
};

int eval(const Board&);
void trace_eval(const Board&, EvalTrace&);

} // namespace azalea
