#include "perft.hpp"
#include "movegeneration.hpp"

#include <cstdint>

namespace azalea {

std::uint64_t perft(const Board& board, int depth) {
    if (depth == 0) return 1;

    Movelist mlist;
    generate_legal_moves(board, mlist);

    std::uint64_t nodes = 0;
    for (const Move& m: mlist) {
        Board copy = board;
        copy.make_move(m);
        nodes += perft(copy, depth-1);
    }

    return nodes;
}

std::vector<DivideEntry> perft_divide(const Board& board, const int depth) {
    std::vector<DivideEntry> entry_vec;

    Movelist mlist;
    generate_legal_moves(board, mlist);

    for (size_t i=0; i<mlist.size(); ++i) {
        DivideEntry de;
        de.move = mlist.moves[i];
        Board copy = board;
        copy.make_move(de.move);
        de.nodes = perft(copy, depth-1);
        entry_vec.push_back(de);
    }

    return entry_vec;
}

} // namespace azalea
