#include "search.hpp"
#include "movegeneration.hpp"

#include <random>

namespace azalea {

void search(Board& board, const SearchLimits& limits, SearchReporter& reporter, std::stop_token st) {
    Movelist mlist;
    generate_legal_moves(board, mlist);

    Move best{};
    if (mlist.size() > 0) {
        thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<std::size_t> pick(0, mlist.size() - 1);
        best = mlist.moves[pick(rng)];
    }

    // "go infinite": the answer may only come after "stop"
    while (limits.infinite and not st.stop_requested())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    reporter.on_bestmove(best, Move{});
}

} // namespace azalea
