#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <time.h>
#include <iomanip>

#include "params.hpp"
#include "statistics.hpp"
#include "board.hpp"
#include "fen.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "util.hpp"
#include "eval.hpp"
#include "search.hpp"
#include "zobrist.hpp"


// tt test section
TTentry	tTable[ttsize];
// end of tt test section

int movetime; // movetime in ms

int main(int argc, char* argv[]) {
    if (argc > 1) {
	if (std::string(argv[1]) == "-v"
				    or std::string(argv[1]) == "--version") {
	    std::cout << "Azalea " << azalea::majorVersion << "."
				    << azalea::minorVersion << "."
				    << azalea::patchVersion
				    << azalea::addVersion << "\n"
			<< "by Dominik Steinmetz\n";
	    return 0;
	} else if (std::string(argv[1]) == "-t"
		    or std::string(argv[1]) == "--test") {
	    //runTests();
	    std::cout << "no longer supported\n";
	    return 0;
	}
    }

    std::cout << "Azalea " << azalea::majorVersion << "."
		      << azalea::minorVersion << "."
		      << azalea::patchVersion
		      << azalea::addVersion << " <3\n";
    // TO BE DELETED
    std::cout << "current size of transposition table: "
	<< (ttsize*sizeof(TTentry)/1000/1000) << " MB\n";

    const zobristKeys zobrist = initZobrist();

    // delete old statfile in case of restart
    if constexpr (azalea::statistics) deleteStatfile();
    // print boot message to statfile
    if constexpr (azalea::statistics) dumpBootMsg();

    GameState gs
	= fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		zobrist);

    bool inCheck = false;
    UnmakeInfo umi;

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

	// transposition table entry
	} else if (command == "ttentry") {
	    std::cout << "TRANSPOSITION TABLE ENTRY:\n";
	    const auto entry = tTable[gs.zhash%(ttsize)];
	    std::cout << "\thash: " << entry.zhash << "\n";
	    std::cout << "\tbestmove: " << entry.bestmove << "\n";
	    std::cout << "\tdraft: " << entry.draft << "\n";
	    std::cout << "\tscore: " << entry.score << "\n";
	    if (entry.nodeType == NodeType::AlphaNode) std::cout << "\tnode type: alpha node\n";
	    if (entry.nodeType == NodeType::BetaNode) std::cout << "\tnode type: beta node\n";
	    if (entry.nodeType == NodeType::PVNode) std::cout << "\tnode type: PV node\n";

	// hash
	} else if (command == "hash") {
	    std::cout << "dec: " << gs.zhash << "\n";
	    std::cout << "hex: " << std::hex << gs.zhash << std::dec << "\n";

	// fen
	} else if (command == "fen") {
	    std::cout << "FEN: " <<  toFen(gs) << "\n";

	// eval
	} else if (command == "eval") {
	    eval<true>(gs);

	// move
	} else if (command.substr(0, 5) == "move ") {
	    std::vector<Move> ml;
	    generateLegalMoves(gs, ml, inCheck);
	    std::string mstr = command.substr(5);
	    bool legal = false;
	    for (const auto& m: ml) {
		if (mstr == toString(m)) {
		    umi = gs.makeMove(m, zobrist);
		    legal = true;
		}
	    }
	    if (not legal) std::cout << "illegal move!\n";

	// unmake move
	} else if (command == "unmake") {
	    gs.unmakeMove(umi, zobrist);

	// legal moves
	} else if (command == "legalmoves") {
	    std::vector<Move> ml;
	    generateLegalMoves(gs, ml, inCheck);
	    for (const auto& m: ml) {
		std::cout << m << "  ";
	    }
	    std::cout << "\nnumber of moves: " << ml.size() << std::endl;

	// uci standard stuff
	} else if (command == "ucinewgame") {
	    gs =
	    fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		    zobrist);
	} else if (command == "isready") {
	    std::cout << "readyok" << std::endl;

	// position section
	} else if (command.substr(0, 8) == "position") {
	    if (command.substr(9, 8) == "startpos") {
		gs = fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
			    " w KQkq - 0 1", zobrist);
	    } else if (command.substr(9, 3) == "fen") {
		gs = fen(command.substr(13), zobrist);
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
		    	generateLegalMoves(gs, ml, inCheck);
		    	for (const auto& m: ml) {
				    if (mstring == toString(m)) {
				    gs.makeMove(m, zobrist);
				}
			    }
			}
	    }

	// go section
	} else if (command.substr(0, 2) == "go") {
	    if (command.substr(3, 5) == "perft") {
		const int depth
			= std::stoi(command.substr(9, command.length()));
		const unsigned long long int n = perft(gs, depth, zobrist);
		std::cout << "nodes: " << n << "\n";
		//perftdiv(gs, depth, zobrist);
	    } else if (command.substr(3, 5) == "depth") {
		const int depth
			    = std::stoi(command.substr(9, command.length()));
		movetime = -1;
		search(gs, depth, zobrist);
	    } else if (command.substr(3, 9) == "infinite") {
		movetime = -1;
		search(gs, azalea::maxDepth, zobrist);
	    } else if (command.substr(3, 8) == "movetime") {
		movetime = std::stoi(command.substr(12, command.length()));
		search(gs, azalea::maxDepth, zobrist);
	    } else {
			// just make a random move
			std::vector<Move> ml;
			generateLegalMoves(gs, ml, inCheck);
			srand(time(NULL));
			const int random = rand() % ml.size();
			std::cout << "bestmove " << ml[random] << std::endl;
	    }
	} else if (command.substr(0, 3) == "uci") {
	    std::cout << "id name Azalea " << azalea::majorVersion << "."
					   << azalea::minorVersion << "."
		      			   << azalea::patchVersion
					   << azalea::addVersion << "\n";
	    std::cout << "id author Dominik Steinmetz\n";
	    std::cout << "uciok\n" << std::flush;
	} else {
	    std::cout << "Unknown command: " << command << "\n" << std::flush;
	}
    }

    return 0;
}
