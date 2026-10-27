#include "engine.hpp"
#include "search.hpp"
#include "perft.hpp"
#include "movegeneration.hpp"

namespace azalea {

void Engine::stop() {
    if (worker.joinable()) {
        worker.request_stop();
        worker.join();
    }
}

void Engine::wait() {
    if (worker.joinable()) {
        worker.join();
    }
}

void Engine::new_game() {
    stop();
    board = Board{start_fen};
}

void Engine::set_position(Board& b) {
    stop();
    board = b;
}

void Engine::start_search(const SearchLimits& limits, SearchReporter& reporter) {
    stop();
    // worker should copy the board once
    worker = std::jthread(
        [board_copy = board, limits, &reporter](std::stop_token st) mutable {
            search(board_copy, limits, reporter, st);
        });
}

std::vector<DivideEntry> Engine::perft_divide(const int depth) {
    return azalea::perft_divide(board, depth);
}

} // namespace azalea
