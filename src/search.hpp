#pragma once

#include "engine.hpp"

#include <cstdint>

namespace azalea {

struct SearchState {
    std::stop_token st;
    std::chrono::steady_clock::time_point start;
    bool timed = false;
    std::chrono::milliseconds soft{}, hard{};
    std::uint64_t node_limit = 0; // go nodes N
    std::uint64_t nodes = 0;
    std::uint64_t next_check = 2048;
    bool stopped = false;

    Move pv_table[MAX_PLY][MAX_PLY];
    int  pv_length[MAX_PLY];

    Move root_best; // TODO: will be deleted once the transposition table is implemented

    std::chrono::milliseconds elapsed() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
    }

    bool should_stop() {
        if (stopped) return true;
        if (nodes > next_check) {
            next_check = nodes + 2048;
            stopped = st.stop_requested()
                or (timed and elapsed() >= hard)
                or (node_limit > 0 and nodes >= node_limit);
        }
        return stopped;
    }
};

void search(Board&, const SearchLimits&, SearchReporter&, std::stop_token);
int alpha_beta(const Board& board, const int depth, const int ply,
               int alpha, int beta, SearchState& state);
int quiescence(const Board& board, const int ply,
               int alpha, int beta, SearchState& state);

} // namespace azalea
