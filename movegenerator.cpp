#include "movegenerator.hpp"

#include <vector>

#include "params.hpp"
#include "statistics.hpp"
#include "board.hpp"
#include "move.hpp"

/******************************************************************************
 * Function that generates all legal moves in a given position. Returns the
 * number of legal moves and gives the moves themself on 'movelist'.
 */
void generateLegalMoves(const Board& b, std::vector<Move>& movelist) {
    movelist.resize(0);
    generateKingMoves(b, movelist);
}

void generateKingMoves(const Board& b, std::vector<Move>& movelist) {
    const Color ownColor = (b.whiteToMove ? Color::white : Color::black);
    const Color enemyColor = (b.whiteToMove ? Color::black : Color::white);

    const Piece ownKing = (b.whiteToMove ? b.whiteKing : b.blackKing);
    const int kingIndex = ownKing.location;

    // generate king danger and out-of-board detection
    // squares in question:
    bool free_n = true;
    bool free_s = true;
    bool free_e = true;
    bool free_w = true;
    bool free_nw = true;
    bool free_ne = true;
    bool free_sw = true;
    bool free_se = true;

    // if north if out of board, so is nw and ne
    if (b.mailbox[index+10].type == PieceType::outOfBoard) {
	free_n = false;
	free_nw = false;
	free_ne = false;
    }

    // if south if out of board, so is sw and se
    if (b.mailbox[index-10].type == PieceType::outOfBoard) {
	free_s = false;
	free_sw = false;
	free_se = false;
    }

    // if east if out of board, so is ne and se
    if (b.mailbox[index+1].type == PieceType::outOfBoard) {
	free_e = false;
	free_se = false;
	free_ne = false;
    }

    // if west if out of board, so is sw and nw
    if (b.mailbox[index-1].type == PieceType::outOfBoard) {
	free_w = false;
	free_sw = false;
	free_nw = false;
    }

    // king danger north
    // TODO


    // east
    if ((b.mailbox[kingIndex+1].type==PieceType::empty
	    or b.mailbox[kingIndex+1].color==enemyColor)
	    and b.mailbox[kingIndex+1].type!=PieceType::outOfBoard)
    movelist.push_back(
	    Move(kingIndex, kingIndex+1,
		 (b.mailbox[kingIndex+1].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // west
    if ((b.mailbox[kingIndex-1].type==PieceType::empty
	    or b.mailbox[kingIndex-1].color==enemyColor)
	    and b.mailbox[kingIndex-1].type!=PieceType::outOfBoard)
    movelist.push_back(
	    Move(kingIndex, kingIndex-1,
		 (b.mailbox[kingIndex-1].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // north
    if ((b.mailbox[kingIndex+10].type==PieceType::empty
	    or b.mailbox[kingIndex+10].color==enemyColor)
	    and b.mailbox[kingIndex+10].type!=PieceType::outOfBoard)
    movelist.push_back(
	    Move(kingIndex, kingIndex+10,
		 (b.mailbox[kingIndex+10].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // south
    if ((b.mailbox[kingIndex-10].type==PieceType::empty
	    or b.mailbox[kingIndex-10].color==enemyColor)
	    and b.mailbox[kingIndex-10].type!=PieceType::outOfBoard)
    movelist.push_back(
	    Move(kingIndex, kingIndex-10,
		 (b.mailbox[kingIndex-10].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // north east
    if ((b.mailbox[kingIndex+11].type==PieceType::empty
	    or b.mailbox[kingIndex+11].color==enemyColor)
	    and b.mailbox[kingIndex+11].type!=PieceType::outOfBoard)
    movelist.push_back(
	    Move(kingIndex, kingIndex+11,
		 (b.mailbox[kingIndex+11].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // south west
    if ((b.mailbox[kingIndex-11].type==PieceType::empty
	    or b.mailbox[kingIndex-11].color==enemyColor)
	    and b.mailbox[kingIndex-11].type!=PieceType::outOfBoard)
    movelist.push_back(
	    Move(kingIndex, kingIndex-11,
		 (b.mailbox[kingIndex-11].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // north west
    if ((b.mailbox[kingIndex+9].type==PieceType::empty
	    or b.mailbox[kingIndex+9].color==enemyColor)
	    and b.mailbox[kingIndex+9].type!=PieceType::outOfBoard)
    movelist.push_back(
	    Move(kingIndex, kingIndex+9,
		 (b.mailbox[kingIndex+9].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // south east
    if ((b.mailbox[kingIndex-9].type==PieceType::empty
	    or b.mailbox[kingIndex-9].color==enemyColor)
	    and b.mailbox[kingIndex-9].type!=PieceType::outOfBoard)
    movelist.push_back(
	    Move(kingIndex, kingIndex-9,
		 (b.mailbox[kingIndex-9].type==PieceType::empty),
		 false, false, false, PieceType::empty));
}
