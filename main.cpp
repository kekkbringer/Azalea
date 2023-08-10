#include <iostream>
#include <fstream>
#include <vector>

#include "params.hpp"
#include "statistics.hpp"
#include "board.hpp"
#include "fen.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "util.hpp"

int main(int argc, char* argv[]) {
    std::cout << "Azalea " << azalea::majorVersion << "."
		      << azalea::minorVersion << "."
		      << azalea::patchVersion << " <3\n";

    // delete old statfile in case of restart
    if constexpr (azalea::statistics) deleteStatfile();
    // print boot message to statfile
    if constexpr (azalea::statistics) dumpBootMsg();

    GameState gs
	= fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

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
	} else if (command.substr(0, 3) == "uci") {
	    std::cout << "id name Azalea " << azalea::majorVersion << "."
					   << azalea::minorVersion << "."
		      			   << azalea::patchVersion << "\n";
	    std::cout << "id author Dominik Steinmetz\n";
	    std::cout << "uciok\n" << std::flush;
	} else {
	    std::cout << "Unknown command: " << command << "\n" << std::flush;
	}
    }

    return 0;
}
