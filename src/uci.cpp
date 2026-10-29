#include "uci.hpp"
#include "version.hpp"
#include "board.hpp"
#include "movegeneration.hpp"
#include "eval.hpp"

#include <string>
#include <sstream>
#include <algorithm>

namespace azalea {

Uci::Uci(Engine& engine_, std::istream& in_, std::ostream& out_)
    : in(in_), out(out_), engine(engine_) {
    send("Azalea version " + std::string(version()));
}

void Uci::send(const std::string_view line) {
    std::scoped_lock lock(out_mutex);
    out << line << '\n';
    out.flush();
}

void Uci::loop() {
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "quit") {
            break;

        } else if (cmd == "uci") {
            send("id name Azalea " + std::string(version()));
            send("id author Dominik Steinmetz");
            send("option name Hash type spin default 16 min 1 max 1024");
            send("uciok");

        } else if (cmd == "ucinewgame") {
            engine.stop();
            engine.new_game();

        } else if (cmd == "isready") {
            send("readyok");

        } else if (cmd == "setoption") {
            handle_option(iss);

        } else if (cmd == "position") {
            handle_position(iss);

        } else if (cmd == "go") {
            handle_go(iss);

        } else if (cmd == "stop") {
            engine.stop();

        } else if (cmd == "ponderhit") {
            send("info string not yet implemented");
        }
    }

    engine.stop();
}

void Uci::handle_position(std::istringstream& iss) {
    std::string token;
    iss >> token;

    std::string fen;
    if (token == "startpos") {
        fen = start_fen;
        iss >> token; // "moves" if present
    } else if (token == "fen") {
        while (iss >> token and token != "moves") {
            fen += token + ' ';
        }
    } else return; // malformed command, do nothing

    Board board(fen);
    while (iss >> token) {
        Move m = parse_move(board, token);
        if (m == Move()) return; // illegal move, do nothing
        board.make_move(m);
    }

    engine.set_position(board);
}

void Uci::handle_go(std::istringstream& iss) {
    SearchLimits sl;
    std::string token;

    while (iss >> token) {
        if (token == "depth") iss >> sl.depth;
        else if (token == "nodes") iss >> sl.nodes;
        else if (token == "movestogo") iss >> sl.moves_to_go;
        else if (token == "infinite") sl.infinite = true;
        else if (token == "wtime") sl.wtime = read_ms(iss);
        else if (token == "btime") sl.btime = read_ms(iss);
        else if (token == "winc") sl.winc = read_ms(iss);
        else if (token == "binc") sl.binc = read_ms(iss);
        else if (token == "movetime") sl.movetime = read_ms(iss);

        else if (token == "perft") {
            int depth = 0;
            iss >> depth;
            if (depth < 1) return;
            uint64_t total = 0;
            const auto divide_moves = engine.perft_divide(depth);
            for (const auto& [move, nodes]: divide_moves) {
                send(to_uci(move) + ": " + std::to_string(nodes));
                total += nodes;
            }
            send("");
            send("Nodes searched: " + std::to_string(total));
            return;
        }
    }

    engine.start_search(sl, *this);
}

Move Uci::parse_move(Board& board, std::string_view text) {
    Movelist move_list;
    generate_legal_moves(board, move_list);

    for (const Move& m: move_list) {
        if (to_uci(m) == text) return m;
    }

    return Move();
}

std::chrono::milliseconds Uci::read_ms(std::istringstream& iss) {
    long long ms = 0;
    iss >> ms;
    return std::chrono::milliseconds{ms};
}

std::string Uci::format_score(const int s) {
    if (s >=  MATE_BOUND) return "mate " + std::to_string((SCORE_MATE - s + 1) / 2);
    if (s <= -MATE_BOUND) return "mate " + std::to_string(-(SCORE_MATE + s) / 2);
    return "cp " + std::to_string(s);
}

void Uci::on_info(const SearchInfo& si) {
    std::string pv_line = " pv ";
    for (const auto& m: si.pv) pv_line += to_uci(m) + " ";
    send("info depth "    + std::to_string(si.depth)
           + " seldepth " + std::to_string(si.seldepth)
           + " score "    + format_score(si.score_cp)
           + " nodes "    + std::to_string(si.nodes)
           + " nps "      + std::to_string(si.nps)
           + " hashfull " + std::to_string(si.hashfull)
           + " time "     + std::to_string(si.time.count())
           + pv_line);
}

void Uci::on_bestmove(const Move& best, const Move& ponder) {
    std::string line = "bestmove " + (best == Move() ? std::string("0000") : to_uci(best));
    if (ponder != Move()) line += " ponder " + to_uci(ponder);
    send(line);
}

void Uci::handle_option(std::istringstream& iss) {
    std::string token, name, value;
    iss >> token; // "name"
    while (iss >> token and token != "value")
        name += (name.empty() ? "" : " ") + token;
    while (iss >> token)
        value += (value.empty() ? "" : " ") + token;

    if (name == "Hash") {
        try {
            const auto mb = std::clamp(std::stoi(value), 1, 1024);
            engine.set_hash_mb(static_cast<std::size_t>(mb));
        } catch (const std::exception&) {
            send("info string invalid value for Hash: " + value);
        }
    } else {
        send("info string unknown option " + name);
    }
}

} // namespace azalea
