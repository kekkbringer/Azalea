#include <iostream>
#include <fstream>
#include <vector>

#include "params.hpp"
#include "statistics.hpp"
#include "board.hpp"
#include "fen.hpp"
#include "move.hpp"
#include "movegenerator.hpp"

int main(int argc, char* argv[]) {
    // delete old statfile in case of restart
    if constexpr (azalea::statistics) deleteStatfile();
    // print boot message to statfile
    if constexpr (azalea::statistics) dumpBootMsg();

    Board b = fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::vector<Move> movelist;
    generateLegalMoves(b, movelist);
    std::cout << "number of legal moves: " << movelist.size() << "\n";

    return 0; // TO BE DELETED

    /**************************************************************************
     * interactive UCI section
     *
     * known commands:
     * 		- quit
     * 		- go <>
     * 			perft
     */
    for (std::string command; std::getline(std::cin, command);) {
	// quit section
	if (command == "quit") {
	    return 0;

	// go section
	} else if (command.substr(0, 2) == "go") {
	    if (command.substr(3, 5) == "perft") {
		const int depth
			    = std::stoi(command.substr(9, command.length()));
		std::cout << "no perft implemented yet... :(\n" << std::flush;
	    }
	}
    }

    return 0;
}
