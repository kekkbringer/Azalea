#include "eval.hpp"

#include <bit>

namespace azalea {

/**
 * Evaluates the board in centipawns from the perspective of the side to move.
 */
int eval(const Board& b) {
    int score = 0;

    score += PAWN_EVAL   * (std::popcount(b.white_pieces & b.pawns)
                          - std::popcount(b.black_pieces & b.pawns))
           + KNIGHT_EVAL * (std::popcount(b.white_pieces & b.knights)
                          - std::popcount(b.black_pieces & b.knights))
           + BISHOP_EVAL * (std::popcount(b.white_pieces & b.bishops)
                          - std::popcount(b.black_pieces & b.bishops))
           + ROOK_EVAL   * (std::popcount(b.white_pieces & b.rooks)
                          - std::popcount(b.black_pieces & b.rooks))
           + QUEEN_EVAL  * (std::popcount(b.white_pieces & b.queens)
                          - std::popcount(b.black_pieces & b.queens));

    return b.white_to_move ? score : -score;
}

} // namespace azalea
