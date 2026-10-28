#pragma once

#include "board.hpp"
#include "move.hpp"
#include "movegeneration.hpp"

#include <array>
#include <algorithm>

namespace azalea {

class MovePicker {
public:
    // main search -> all moves
    MovePicker(const Board& b, Move ttm, const std::array<Move, 2>& ks)
        : board(b), tt_move(ttm), killers(ks) {
            generate_pseudolegal_moves<GenType::All>(board, movelist);
            score_all();
        }

    // quiescense search -> only captures (and no killers)
    // when in check -> search all moves (evasions)
    MovePicker(const Board& b, Move ttm)
        : board(b), tt_move(ttm) {
            if (board.in_check()) generate_pseudolegal_moves<GenType::All>(board, movelist);
            else generate_pseudolegal_moves<GenType::Captures>(board, movelist);
            score_all();
        }

    // returns the next pseudolegal move (nullmove is list is exhausted)
    [[nodiscard]] Move next() {
        if (cur == size) return Move{};

        std::size_t best = cur;
        for (std::size_t i=cur+1; i<size; ++i)
            if (scores[i] > scores[best]) best = i;

        std::swap(movelist.moves[cur], movelist.moves[best]);
        std::swap(scores[cur], scores[best]);

        return movelist.moves[cur++];
    }

private:
    // score all moves per MVV LVA
    void score_all() {
        size = movelist.size();
        for (std::size_t i=0; i<size; ++i)
            scores[i] = score(movelist.moves[i]);
    }

    // MVV LVA implementation
    int score(Move m) const {
        if (m == tt_move and tt_move != Move{}) return 1'000'000;

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

        // killer moves
        if (m == killers[0]) return 90'000;
        if (m == killers[1]) return 80'000;

        // non-killer quiet moves, not ordered yet
        return 0;
    }


    const Board& board;
    Move tt_move;
    std::array<Move, 2> killers{}; // empty in quiescense search
    Movelist movelist;
    std::array<int, 256> scores{};
    std::size_t cur  = 0;
    std::size_t size = 0;
};

} // namespace azalea
