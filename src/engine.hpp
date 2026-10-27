#pragma once

#include "board.hpp"
#include "perft.hpp"
#include "transposition.hpp"

#include <chrono>
#include <mutex>
#include <thread>

namespace azalea {

constexpr int MAX_PLY = 256;

constexpr std::string_view start_fen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct SearchLimits {
    int depth = 0; // 0 means unlimited
    std::uint64_t nodes = 0;
    std::chrono::milliseconds wtime{}, btime{}, winc{}, binc{}, movetime{};
    int moves_to_go = 0;
    bool infinite = false;
};

/**
 * struct for one "info" line reported by a SearchReporter.
 */
struct SearchInfo {
    int depth;
    int score_cp;
    std::uint64_t nodes;
    std::chrono::milliseconds time;
    std::vector<Move> pv;
};

/**
 * parent struct for different reporter such as UCI or internal testing interfaces.
 */
struct SearchReporter {
    virtual ~SearchReporter() = default;
    virtual void on_info(const SearchInfo&) = 0;
    virtual void on_bestmove(const Move& best, const Move& ponder) = 0;
};

/**
 * Main wrapper class for the whole chess engine.
 */
class Engine {
public:
    void new_game();
    void set_position(Board&);
    void set_hash_mb(std::size_t);
    void start_search(const SearchLimits&, SearchReporter&);
    std::vector<DivideEntry> perft_divide(const int);
    void stop();
    void wait();

private:
    TranspositionTable transposition_table;
    Board board{start_fen};
    std::jthread worker;
};

} // namespace azalea
