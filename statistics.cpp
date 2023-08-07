#include <string>
#include <fstream>
#include <cstdio>

#include "params.hpp"
#include "board.hpp"

#include "statistics.hpp"


/******************************************************************************
 * Function that deletes the statistics file give in 'azalea::statfile',
 * defaul name is 'stats.aza'.
 */
void deleteStatfile() {
    std::remove(azalea::statfile);
}

/******************************************************************************
 * Simple function that dumps a string given in 'text' to the statistics file
 * 'azalea::stat_file', default name is 'stats.aza'.
 */
void outputStats(const std::string text) {
    std::ofstream file;
    file.open(azalea::statfile, std::ios_base::app);
    file << text;
    file.close();
}

/******************************************************************************
 * Function that dumps boot message to statfile, containing version info
 */
void dumpBootMsg() {
    std::ofstream file;
    file.open(azalea::statfile, std::ios_base::app);
    file << "booting Azalea " << azalea::majorVersion << "."
		      << azalea::minorVersion << "."
		      << azalea::patchVersion << " <3\n";
    file.close();
}

/******************************************************************************
 * Auxiliary function that converts a 'PieceInfo' to a single character for
 * printing purposes.
 */
char pieceToChar(const PieceInfo& pinfo) {
    if (pinfo.color == Color::white) { // white
	switch (pinfo.type) {
	    case PieceType::pawn:
		return 'P';
	    case PieceType::knight:
		return 'N';
	    case PieceType::bishop:
		return 'B';
	    case PieceType::rook:
		return 'R';
	    case PieceType::queen:
		return 'Q';
	    case PieceType::king:
		return 'K';
	    case PieceType::empty:
		return '.';
	    case PieceType::outOfBoard:
		return ' ';
	}
    } else { // black
	switch (pinfo.type) {
	    case PieceType::pawn:
		return 'p';
	    case PieceType::knight:
		return 'n';
	    case PieceType::bishop:
		return 'b';
	    case PieceType::rook:
		return 'r';
	    case PieceType::queen:
		return 'q';
	    case PieceType::king:
		return 'k';
	    case PieceType::empty:
		return '.';
	    case PieceType::outOfBoard:
		return ' ';
	}
    }
    return 'x';
}

/******************************************************************************
 * Function that dumps the state of 'board' to statfile.
 */
void dumpBoard(const Board& b) {
    std::ofstream file;
    file.open(azalea::statfile, std::ios_base::app);

    file << "board state:\n";

    for (int i=0; i<12; i++) {
	file << "   ";
	for (int j=0; j<10; j++) {
	    file << pieceToChar(b.mailbox[10*i+j]) << " ";
	}
	file << "\n";
    }
    file << " whiteToMove: " << b.whiteToMove << "\n";
    file << " whiteShort: " << b.whiteShort << "\n";
    file << " whiteLong: " << b.whiteLong << "\n";
    file << " blackShort: " << b.blackShort << "\n";
    file << " blackLong: " << b.blackLong << "\n";
    file << " enPassantTarget: " << b.enPassantTarget << "\n";
    file << " halfmoveClock: " << b.halfmoveClock << "\n";

    file.close();
}
