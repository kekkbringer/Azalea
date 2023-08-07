#include "fen.hpp"

#include <string>
#include <iostream> // to be deleted

#include "params.hpp"
#include "statistics.hpp"
#include "board.hpp"
#include "util.hpp"

/******************************************************************************
 * Function that reads in a fen string and returns the corresponding board.
 */
Board fen(std::string f) {
    if constexpr (azalea::statistics)
	outputStats("called function 'fen' with string " + f + "\n");
    Board b = Board();

    auto it = f.begin();

    // piece placement
    int index = 21;
    int rank = 2;
    for (;;it++) {
	const char c = *it;
	switch (c) {
	    case 'P':
		b.mailbox[index] = PieceInfo(Color::white, PieceType::pawn);
		index++;
		break;
	    case 'N':
		b.mailbox[index] = PieceInfo(Color::white, PieceType::knight);
		index++;
		break;
	    case 'B':
		b.mailbox[index] = PieceInfo(Color::white, PieceType::bishop);
		index++;
		break;
	    case 'R':
		b.mailbox[index] = PieceInfo(Color::white, PieceType::rook);
		index++;
		break;
	    case 'Q':
		b.mailbox[index] = PieceInfo(Color::white, PieceType::queen);
		index++;
		break;
	    case 'K':
		b.mailbox[index] = PieceInfo(Color::white, PieceType::king);
		index++;
		break;
	    case 'p':
		b.mailbox[index] = PieceInfo(Color::black, PieceType::pawn);
		index++;
		break;
	    case 'n':
		b.mailbox[index] = PieceInfo(Color::black, PieceType::knight);
		index++;
		break;
	    case 'b':
		b.mailbox[index] = PieceInfo(Color::black, PieceType::bishop);
		index++;
		break;
	    case 'r':
		b.mailbox[index] = PieceInfo(Color::black, PieceType::rook);
		index++;
		break;
	    case 'q':
		b.mailbox[index] = PieceInfo(Color::black, PieceType::queen);
		index++;
		break;
	    case 'k':
		b.mailbox[index] = PieceInfo(Color::black, PieceType::king);
		index++;
		break;
	    case '1':
		index += 1;
		break;
	    case '2':
		index += 2;
		break;
	    case '3':
		index += 3;
		break;
	    case '4':
		index += 4;
		break;
	    case '5':
		index += 5;
		break;
	    case '6':
		index += 6;
		break;
	    case '7':
		index += 7;
		break;
	    case '8':
		index += 8;
		break;
	    case '/':
		rank++;
		index = rank*10 + 1;
		break;
	    case ' ':
		goto sidetomove;
	}
    }

    // side to move
    sidetomove:
    it++; // blank space
    char c = *it++;
    if (c=='b') b.whiteToMove = false;

    // castling rights
    it++; // blank space
    for (;;it++) {
	c = *it;
	switch (c) {
	    case 'K':
		b.whiteShort = true;
		break;
	    case 'Q':
		b.whiteLong = true;
		break;
	    case 'k':
		b.blackShort = true;
		break;
	    case 'q':
		b.blackLong = true;
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
	b.enPassantTarget = toIndex(ep);
    } 

    // halfmove clock
    it++; // blank space
    b.halfmoveClock = *++it - '0';
    if (*++it != ' ') {
	b.halfmoveClock *= 10;
	b.halfmoveClock += *it - '0';
    }

    // debug output
    if constexpr (azalea::statistics) dumpBoard(b);

    return b;
}
