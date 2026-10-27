#pragma once

#include "move.hpp"

#include <vector>
#include <cstddef>
#include <cstdint>

namespace azalea {

/**
 * Entry in the transposition table.
 */
struct TTentry {
    std::uint64_t hash;
    Move move;
    int score;
    int depth;
    enum Bound { Exact, Lower, Upper } bound;
};

class TranspositionTable {
public:
    explicit TranspositionTable(std::size_t mb = 16) { resize(mb); }
    void resize(const std::size_t mb);
    void clear();
    const TTentry* probe(const std::uint64_t hash) const; // nullptr if not found
    void store(std::uint64_t hash, Move, int score, int depth, TTentry::Bound);
private:
    std::size_t size;
    std::vector<TTentry> entries;

    std::size_t index(std::uint64_t hash) const { return hash % size; }
};

} // namespace azalea
