#pragma once

namespace azalea {

enum TraceIndex : int {
    TEMPO,
    PAWN_EVAL, KNIGHT_EVAL, BISHOP_EVAL, ROOK_EVAL, QUEEN_EVAL, // 5
    PASSED_PAWN_START,                                          // 8
    PASSED_PAWN_KING_DISTANCE = PASSED_PAWN_START + 8,          // 8
    ISOLATED_PAWN = PASSED_PAWN_KING_DISTANCE + 8,
    DOUBLED_PAWN,
    PST_START,                                                  // 6 pieces * 64 squares
    BISHOP_PAIR = PST_START + 6 * 64,
    ROOK_HALF_OPEN,
    ROOK_OPEN,
    N_TRACE
};

struct EvalTrace {
    int early[N_TRACE]{};
    int late[N_TRACE]{};
    int phase = 0;
};

} // namespace azalea
