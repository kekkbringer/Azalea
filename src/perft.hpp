#pragma once

#include "board.hpp"
#include "movegeneration.hpp"

#include <vector>

namespace azalea {

struct DivideEntry {
    Move move;
    std::uint64_t nodes;
};

std::uint64_t perft(const Board& board, int depth);
std::vector<DivideEntry> perft_divide(const Board& board, const int depth);

} // namespace azalea
