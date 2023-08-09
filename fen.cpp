#include "fen.hpp"

#include <string>

#include "params.hpp"
#include "statistics.hpp"
#include "board.hpp"
#include "util.hpp"

/******************************************************************************
 * Function that reads in a fen string and returns the corresponding board.
 */
GameState fen(std::string f) {
    if constexpr (azalea::statistics)
	outputStats("called function 'fen' with string " + f + "\n");
    Board b = Board();
    GameState gs;
    gs.whiteToMove = true;
    gs.whiteShort = false;
    gs.whiteLong = false;
    gs.blackShort = false;
    gs.blackLong = false;
    gs.halfmoveClock = 0;
    gs.epTarget = -1;

    auto it = f.begin();

    // piece placement
    int index = 63;
    for (;;it++) {
	const char c = *it;
	switch (c) {
	    case 'P':
		b.wPawns |= (1ULL << index);
		index--;
		break;
	    case 'N':
		b.wKnights |= (1ULL << index);
		index--;
		break;
	    case 'B':
		b.wBishops |= (1ULL << index);
		index--;
		break;
	    case 'R':
		b.wRooks |= (1ULL << index);
		index--;
		break;
	    case 'Q':
		b.wQueens |= (1ULL << index);
		index--;
		break;
	    case 'K':
		b.wKing |= (1ULL << index);
		index--;
		break;
	    case 'p':
		b.bPawns |= (1ULL << index);
		index--;
		break;
	    case 'n':
		b.bKnights |= (1ULL << index);
		index--;
		break;
	    case 'b':
		b.bBishops |= (1ULL << index);
		index--;
		break;
	    case 'r':
		b.bRooks |= (1ULL << index);
		index--;
		break;
	    case 'q':
		b.bQueens |= (1ULL << index);
		index--;
		break;
	    case 'k':
		b.bKing |= (1ULL << index);
		index--;
		break;
	    case '1':
		index--;
		break;
	    case '2':
		index -= 2;
		break;
	    case '3':
		index -= 3;
		break;
	    case '4':
		index -= 4;
		break;
	    case '5':
		index -= 5;
		break;
	    case '6':
		index -= 6;
		break;
	    case '7':
		index -= 7;
		break;
	    case '8':
		index -= 8;
		break;
	    case '/':
		index--;
		break;
	    case ' ':
		goto sidetomove;
	}
    }
    gs.board = b;

    // side to move
    sidetomove:
    it++; // blank space
    char c = *it++;
    if (c=='b') gs.whiteToMove = false;

    // castling rights
    it++; // blank space
    for (;;it++) {
	c = *it;
	switch (c) {
	    case 'K':
		gs.whiteShort = true;
		break;
	    case 'Q':
		gs.whiteLong = true;
		break;
	    case 'k':
		gs.blackShort = true;
		break;
	    case 'q':
		gs.blackLong = true;
		break;
	    case '-':
		it++;
		goto enpassant;
	    case ' ':
		goto enpassant;
	}
    }

    // en passant target
    enpassant:
    it++; // blank space
    if (*it != '-') {
	std::string ep(1, *it++);
	ep += *it++;
	gs.epTarget = toIndex(ep);
    } 

    // halfmove clock
    it++; // blank space
    gs.halfmoveClock = *++it - '0';
    if (*++it != ' ') {
	gs.halfmoveClock *= 10;
	gs.halfmoveClock += *it - '0';
    }

    return gs;
}
