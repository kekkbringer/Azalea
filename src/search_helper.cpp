#include "search_helper.hpp"

namespace azalea {

int score_move_mvv_lva(const Board& board, const Move& m, const Move& hint) {
    if (m == hint) return 1'000'000; // PV or TT move: always first

    if (m.is_capture()) {
        const auto victim = m.is_en_passant() ? PieceType::Pawn : board.piece_at_square(m.to());
        const auto attacker = board.piece_at_square(m.from());

        int score = 100'000;
        switch (victim) {
            case PieceType::Pawn:   { score += 10*1; break; }
            case PieceType::Knight: { score += 10*2; break; }
            case PieceType::Bishop: { score += 10*3; break; }
            case PieceType::Rook:   { score += 10*4; break; }
            case PieceType::Queen:  { score += 10*5; break; }
            default: ;
        }
        switch (attacker) {
            case PieceType::Pawn:   { score -= 1; break; }
            case PieceType::Knight: { score -= 2; break; }
            case PieceType::Bishop: { score -= 3; break; }
            case PieceType::Rook:   { score -= 4; break; }
            case PieceType::Queen:  { score -= 5; break; }
            case PieceType::King:   { score -= 6; break; }
            default: ;
        }

        if (m.is_promotion()) score += 1'000;
        return score;
    } // capture

    if (m.is_promotion()) {
        int score = 90'000;
        if (m.is_knight_promotion()) score += 2;
        else if (m.is_bishop_promotion()) score += 3;
        else if (m.is_rook_promotion()) score += 4;
        else if (m.is_queen_promotion()) score += 5;
        return score;
    }

    // quiet moves, not ordered yet
    return 0;
}

} // namespace azalea
