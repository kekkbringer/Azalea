#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

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

	// legal moves
	} else if (command == "legalmoves") {
	    std::vector<Move> ml;
	    generateLegalMoves(gs, ml);
	    for (const auto& m: ml) {
		std::cout << m << "  ";
	    }
	    std::cout << "\nnumber of moves: " << ml.size() << std::endl;

	// uci standard stuff
	} else if (command == "ucinewgame") {
	} else if (command == "isready") {
	    std::cout << "readyok" << std::endl;

	// position section
	} else if (command.substr(0, 8) == "position") {
	    if (command.substr(9, 8) == "startpos") {
		gs = fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
							    " w KQkq - 0 1");
	    } else if (command.substr(9, 3) == "fen") {
		gs = fen(command.substr(13));
		//gs = fen(command.substr(13).c_str());
	    } else {
		std::cout << "Unknown command: " << command << std::endl;
	    }
	    std::size_t found = command.find("moves");
	    if (found != std::string::npos) {
		std::string moves = command.substr(found+6);
		std::stringstream mm(moves);
		// now actually make these moves
		std::string mstring;
		while (getline(mm, mstring, ' ')) {
		    std::vector<Move> ml;
		    generateLegalMoves(gs, ml);
		    for (const auto& m: ml) {
			if (mstring == toString(m)) {
			    gs.makeMove(m);
			}
		    }
		}
	    }
	// go section
	} else if (command.substr(0, 2) == "go") {
	    if (command.substr(3, 5) == "perft") {
		const int depth
			    = std::stoi(command.substr(9, command.length()));
		//const auto nPerft = perft(gs, depth);
		//std::cout << "nodes searched: " << nPerft << std::endl;
		perftdiv(gs, depth);
	    } else if (command.substr(3, 5) == "depth") {
		const int depth
			    = std::stoi(command.substr(9, command.length()));
		std::vector<Move> ml;
		generateLegalMoves(gs, ml);
		const int random = rand() % ml.size();
		std::cout << "bestmove " << ml[random] << std::endl;
	    } else {
		// just make a random move
		std::vector<Move> ml;
		generateLegalMoves(gs, ml);
		const int random = rand() % ml.size();
		std::cout << "bestmove " << ml[random] << std::endl;
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
