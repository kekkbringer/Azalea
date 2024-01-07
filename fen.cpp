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
GameState fen(std::string f, const zobristKeys& zobrist) {
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
    gs.zhash = 0ULL;

    auto it = f.begin();

    // piece placement
    int index = 63;
    for (;;it++) {
	const char c = *it;
	switch (c) {
	    case 'P':
		gs.board.wPawns |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[0][0][index];
		index--;
		break;
	    case 'N':
		gs.board.wKnights |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[0][1][index];
		index--;
		break;
	    case 'B':
		gs.board.wBishops |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[0][2][index];
		index--;
		break;
	    case 'R':
		gs.board.wRooks |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[0][3][index];
		index--;
		break;
	    case 'Q':
		gs.board.wQueens |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[0][4][index];
		index--;
		break;
	    case 'K':
		gs.board.wKing |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[0][5][index];
		index--;
		break;
	    case 'p':
		gs.board.bPawns |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[1][0][index];
		index--;
		break;
	    case 'n':
		gs.board.bKnights |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[1][1][index];
		index--;
		break;
	    case 'b':
		gs.board.bBishops |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[1][2][index];
		index--;
		break;
	    case 'r':
		gs.board.bRooks |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[1][3][index];
		index--;
		break;
	    case 'q':
		gs.board.bQueens |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[1][4][index];
		index--;
		break;
	    case 'k':
		gs.board.bKing |= (1ULL << index);
		gs.zhash ^= zobrist.pieceSquare[1][5][index];
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
    if (c=='b') {
	gs.whiteToMove = false;
	gs.zhash ^= zobrist.sideToMove;
    }

    // castling rights
    it++; // blank space
    for (;;it++) {
	c = *it;
	switch (c) {
	    case 'K':
		gs.whiteShort = true;
		gs.zhash ^= zobrist.castling[0];
		break;
	    case 'Q':
		gs.whiteLong = true;
		gs.zhash ^= zobrist.castling[1];
		break;
	    case 'k':
		gs.blackShort = true;
		gs.zhash ^= zobrist.castling[2];
		break;
	    case 'q':
		gs.blackLong = true;
		gs.zhash ^= zobrist.castling[3];
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
	gs.zhash ^= zobrist.epFile[fileOf(gs.epTarget)];
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

/******************************************************************************
 * Returns FEN string of current game state
 */
std::string toFen(const GameState& gs) {
    std::string fen = "";

    for (int r=7; r>=0; r--) {
	int empties = 0;
	for (int f=7; f>=0; f--) {
	    const int index = 8*r + f;
	    const uint64_t mask = (1ULL << index);
	    if (mask & gs.board.wPawns) {            if (empties > 0) fen += std::to_string(empties); fen += "P"; empties = 0;
	    } else if (mask & gs.board.wKnights) {   if (empties > 0) fen += std::to_string(empties); fen += "N"; empties = 0;
	    } else if (mask & gs.board.wBishops) {   if (empties > 0) fen += std::to_string(empties); fen += "B"; empties = 0;
	    } else if (mask & gs.board.wRooks) {     if (empties > 0) fen += std::to_string(empties); fen += "R"; empties = 0;
	    } else if (mask & gs.board.wQueens) {    if (empties > 0) fen += std::to_string(empties); fen += "Q"; empties = 0;
	    } else if (mask & gs.board.wKing) {      if (empties > 0) fen += std::to_string(empties); fen += "K"; empties = 0;
	    } else if (mask & gs.board.bPawns) {     if (empties > 0) fen += std::to_string(empties); fen += "p"; empties = 0;
	    } else if (mask & gs.board.bKnights) {   if (empties > 0) fen += std::to_string(empties); fen += "n"; empties = 0;
	    } else if (mask & gs.board.bBishops) {   if (empties > 0) fen += std::to_string(empties); fen += "b"; empties = 0;
	    } else if (mask & gs.board.bRooks) {     if (empties > 0) fen += std::to_string(empties); fen += "r"; empties = 0;
	    } else if (mask & gs.board.bQueens) {    if (empties > 0) fen += std::to_string(empties); fen += "q"; empties = 0;
	    } else if (mask & gs.board.bKing) {      if (empties > 0) fen += std::to_string(empties); fen += "k"; empties = 0;
	    } else {
		empties++;
	    }
	}
	if (empties > 0) fen += std::to_string(empties);
	if (r > 0) fen += "/";
    }

    // side to move
    gs.whiteToMove ? fen+=" w" : fen+=" b";

    // castling
    fen += " ";
    if (gs.whiteShort) fen += "K";
    if (gs.whiteLong)  fen += "Q";
    if (gs.blackShort) fen += "k";
    if (gs.blackLong)  fen += "q";
    if (not (gs.whiteShort or gs.whiteLong or gs.blackShort or gs.blackLong))
	fen += "-";

    // ep target
    fen += " ";
    if (gs.epTarget != -1) fen += std::string(toString(gs.epTarget));
    else fen += "-";

    // halfmove clock and fullmove clock
    fen += " ";
    fen += std::to_string(gs.halfmoveClock);
    fen += " ";
    fen += std::to_string(1 + gs.halfmoveClock/2);

    return fen;
}
