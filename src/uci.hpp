#pragma once

#include "engine.hpp"

#include <iostream>
#include <chrono>

namespace azalea {

class Uci : public SearchReporter {
public:
    Uci(Engine&, std::istream&, std::ostream&);
    void loop();
private:
    void handle_option(std::istringstream&);
    void handle_position(std::istringstream&);
    void handle_go(std::istringstream&);
    void handle_setoption(std::istringstream&);
    void on_info(const SearchInfo&) override; // reports "info depth ..."
    void on_bestmove(const Move&, const Move&) override; // reports "bestmove e2e4 ..."
    void send(const std::string_view line);
    Move parse_move(const Board&, std::string_view);
    static std::chrono::milliseconds read_ms(std::istringstream& iss);
    
    std::istream& in;
    std::ostream& out;
    std::mutex out_mutex; // the search thread also writes
    Engine& engine;
};

} // namespace azalea
