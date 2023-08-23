#include "move.hpp"
#include "board.hpp"
#include "movegenerator.hpp"
#include "util.hpp"

#include <vector>
#include <iostream>
#include <chrono>

void perftdiv(const GameState& gs, int depth) {
    const auto start = std::chrono::high_resolution_clock::now();
    std::vector<Move> ml;
    bool inCheck;
    generateLegalMoves(gs, ml, inCheck);
    unsigned long long int totnodes = 0;
#pragma omp parallel for
    for (const auto& m: ml) {
	auto dummy = gs;
	dummy.makeMove(m);
	const auto nodes = perft(dummy, depth-1);
	std::cout << "  " << m << ": " << nodes << std::endl;
	totnodes += nodes;
    }
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration
	    = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << "total nodes: " << totnodes << "\n";
    std::cout << "total time:  " << duration.count()/1000 << " s\n";
}

unsigned long long int perft(const GameState& gs, int depth) {
    unsigned long long int nodes = 0;

    std::vector<Move> ml;
    bool inCheck;
    generateLegalMoves(gs, ml, inCheck);
    const int nMoves = ml.size();

    if (depth == 1) return (unsigned long long int)nMoves;

    for (const auto& m: ml) {
	auto dummy = gs;
	dummy.makeMove(m);
	nodes += perft(dummy, depth-1);
    }

    return nodes;
}
