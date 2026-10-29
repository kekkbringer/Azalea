#include "transposition.hpp"

#include <algorithm>

namespace azalea {

int TranspositionTable::hashfull() const noexcept {
    int hfull = 0;
    for (std::uint64_t i=0; i<1000; ++i) {
        auto& entry = entries[index(i)];
        if (entry.hash != 0ULL) ++hfull;
    }
    return hfull;
}

void TranspositionTable::resize(const std::size_t mb) {
    size = mb*1024*1024/sizeof(TTentry);
    size = std::max<std::size_t>(1, size);
    entries.assign(size, TTentry{});
}

void TranspositionTable::clear() {
    std::fill(entries.begin(), entries.end(), TTentry{});
}

const TTentry* TranspositionTable::probe(const std::uint64_t hash) const {
    auto& entry = entries[index(hash)];
    if (entry.hash == hash) return &entry;
    return nullptr;
}

void TranspositionTable::store(std::uint64_t hash, Move move, int score, int depth, TTentry::Bound bound) {
    auto& entry = entries[index(hash)];
    // keep old move if the fail-soft framework has no best move to score
    if (move == Move{} and entry.hash == hash) move = entry.move;
    entry = TTentry{hash, move, score, depth, bound};
}

} // namespace azalea
