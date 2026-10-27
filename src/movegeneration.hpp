#pragma once

#include "board.hpp"

#include <array>

namespace azalea {


class Movelist {
public:
    void push(Move m) { moves[size_++] = m; }
    Move* begin() { return moves.data(); }
    Move* end() { return moves.data() + size_; }
    std::size_t size() const { return size_; }
    void remove_at(std::size_t i) { moves[i] = moves[--size_]; }
    std::size_t size_ = 0;
    std::array<Move, 256> moves;
};


void generate_pseudolegal_moves(Board&, Movelist&);
void generate_legal_moves(Board&, Movelist&);

} // namespace azalea
