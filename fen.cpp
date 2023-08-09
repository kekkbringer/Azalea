#include "fen.hpp"

#include <string>
#include <iostream> //to be deleted

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
		gs.board.wPawns |= (1ULL << index);
		index--;
		break;
	    case 'N':
		gs.board.wKnights |= (1ULL << index);
		index--;
		break;
	    case 'B':
		gs.board.wBishops |= (1ULL << index);
		index--;
		break;
	    case 'R':
		gs.board.wRooks |= (1ULL << index);
		index--;
		break;
	    case 'Q':
		gs.board.wQueens |= (1ULL << index);
		index--;
		break;
	    case 'K':
		gs.board.wKing |= (1ULL << index);
		index--;
		break;
	    case 'p':
		gs.board.bPawns |= (1ULL << index);
		index--;
		break;
	    case 'n':
		gs.board.bKnights |= (1ULL << index);
		index--;
		break;
	    case 'b':
		gs.board.bBishops |= (1ULL << index);
		index--;
		break;
	    case 'r':
		gs.board.bRooks |= (1ULL << index);
		index--;
		break;
	    case 'q':
		gs.board.bQueens |= (1ULL << index);
		index--;
		break;
	    case 'k':
		gs.board.bKing |= (1ULL << index);
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
		break;
	    case ' ':
		goto sidetomove;
	}
    }

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

    gs.board.white = gs.board.wPawns | gs.board.wKnights | gs.board.wBishops
		    | gs.board.wRooks | gs.board.wQueens | gs.board.wKing;
    gs.board.black = gs.board.bPawns | gs.board.bKnights | gs.board.bBishops
		    | gs.board.bRooks | gs.board.bQueens | gs.board.bKing;
    gs.board.occ = gs.board.white | gs.board.black;

    return gs;
}
