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
std::pair<Move, int> search_root(const Board&, const int, SearchState&);
int negamax(const Board& board, const int, const int, SearchState&);

} // namespace azalea
