#include "movegenerator.hpp"

#include <vector>

#include "params.hpp"
#include "statistics.hpp"
#include "board.hpp"
#include "move.hpp"
#include "util.hpp"

/******************************************************************************
 * Function that generates all legal moves in a given position. Returns the
 * number of legal moves and gives the moves themself on 'movelist'.
 */
void generateLegalMoves(const Board& b, std::vector<Move>& movelist) {
    movelist.resize(0);
    generateKingMoves(b, movelist); // no castling
    
    // are we in check?
    const auto checker = detectChecker(b);
    if (checker.size() == 2) return; // double check, only king moves no castle
    
    // detect pinned pieces
    const auto pinnedIndicies = detectPinnedPieces(b);

    // if we are in single check do some stupid stuff
    // TODO
    
}

void generateKingMoves(const Board& b, std::vector<Move>& movelist) {
    const Color ownColor = (b.whiteToMove ? Color::white : Color::black);
    const Color enemyColor = (b.whiteToMove ? Color::black : Color::white);

    const Piece ownKing = (b.whiteToMove ? b.whiteKing : b.blackKing);
    const Piece enemyKing = (b.whiteToMove ? b.blackKing : b.whiteKing);

    const int kingIndex = ownKing.location;
    const int enemykingIndex = enemyKing.location;

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
    if (b.mailbox[kingIndex-10].type == PieceType::outOfBoard) {
	free_n = false;
	free_nw = false;
	free_ne = false;
    }

    // if south if out of board, so is sw and se
    if (b.mailbox[kingIndex+10].type == PieceType::outOfBoard) {
	free_s = false;
	free_sw = false;
	free_se = false;
    }

    // if east if out of board, so is ne and se
    if (b.mailbox[kingIndex+1].type == PieceType::outOfBoard) {
	free_e = false;
	free_se = false;
	free_ne = false;
    }

    // if west if out of board, so is sw and nw
    if (b.mailbox[kingIndex-1].type == PieceType::outOfBoard) {
	free_w = false;
	free_sw = false;
	free_nw = false;
    }

    // king danger north
    if (free_n) free_n = !isAttackedNoKing(b, kingIndex - 10);
    if (free_s) free_s = !isAttackedNoKing(b, kingIndex + 10);
    if (free_w) free_w = !isAttackedNoKing(b, kingIndex -  1);
    if (free_e) free_e = !isAttackedNoKing(b, kingIndex +  1);

    if (free_nw) free_nw = !isAttackedNoKing(b, kingIndex - 11);
    if (free_ne) free_ne = !isAttackedNoKing(b, kingIndex -  9);
    if (free_sw) free_sw = !isAttackedNoKing(b, kingIndex +  9);
    if (free_se) free_se = !isAttackedNoKing(b, kingIndex + 11);


    // east
    if (free_e)
    if (b.mailbox[kingIndex+1].type==PieceType::empty
	    or b.mailbox[kingIndex+1].color==enemyColor)
    movelist.push_back(
	    Move(kingIndex, kingIndex+1,
		 (b.mailbox[kingIndex+1].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // west
    if (free_w)
    if (b.mailbox[kingIndex-1].type==PieceType::empty
	    or b.mailbox[kingIndex-1].color==enemyColor)
    movelist.push_back(
	    Move(kingIndex, kingIndex-1,
		 (b.mailbox[kingIndex-1].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // north
    if (free_n)
    if (b.mailbox[kingIndex-10].type==PieceType::empty
	    or b.mailbox[kingIndex-10].color==enemyColor)
    movelist.push_back(
	    Move(kingIndex, kingIndex-10,
		 (b.mailbox[kingIndex-10].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // south
    if (free_s)
    if (b.mailbox[kingIndex+10].type==PieceType::empty
	    or b.mailbox[kingIndex+10].color==enemyColor)
    movelist.push_back(
	    Move(kingIndex, kingIndex+10,
		 (b.mailbox[kingIndex+10].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // north east
    if (free_ne)
    if (b.mailbox[kingIndex-9].type==PieceType::empty
	    or b.mailbox[kingIndex-9].color==enemyColor)
    movelist.push_back(
	    Move(kingIndex, kingIndex-9,
		 (b.mailbox[kingIndex-9].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // south west
    if (free_sw)
    if (b.mailbox[kingIndex+9].type==PieceType::empty
	    or b.mailbox[kingIndex+9].color==enemyColor)
    movelist.push_back(
	    Move(kingIndex, kingIndex+9,
		 (b.mailbox[kingIndex+9].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // north west
    if (free_nw)
    if (b.mailbox[kingIndex-11].type==PieceType::empty
	    or b.mailbox[kingIndex-11].color==enemyColor)
    movelist.push_back(
	    Move(kingIndex, kingIndex-11,
		 (b.mailbox[kingIndex-11].type==PieceType::empty),
		 false, false, false, PieceType::empty));

    // south east
    if (free_se)
    if (b.mailbox[kingIndex+11].type==PieceType::empty
	    or b.mailbox[kingIndex+11].color==enemyColor)
    movelist.push_back(
	    Move(kingIndex, kingIndex+11,
		 (b.mailbox[kingIndex+11].type==PieceType::empty),
		 false, false, false, PieceType::empty));
}
