#include "movegenerator.hpp"

#include <vector>
#include <iostream> // to be deleted

#include "params.hpp"
#include "board.hpp"
#include "move.hpp"
#include "raylookup.hpp"
#include "movegenlookup.hpp"

/******************************************************************************
 * Function that generates all legal moves in a given position, returns them
 * on 'movelist'.
 *
 * Short outline of the procedure:
 * 	- generate the 'kingDanger', all squares attacked by the enemy
 * 	- generate non-castling king moves
 * 	- if in check:
 * 		- double check -> movegen done, only king moves possible
 * 		- single check -> set special check evasion masks
 * 	- detect pinned pieces
 */
void generateLegalMoves(const GameState& gs, std::vector<Move>& movelist) {
    // reset movelist
    movelist.resize(0);

    // reference to board
    const Board& b = gs.board;

    // white to move
    const bool wtm = gs.whiteToMove;

    const bitb enemies = (wtm ? b.black : b.white);

    // calculate king danger
    const bitb kingDanger = generateKingDanger(gs);

    ///////////////////////////////////////////////////////
    //                     king moves                    //
    ///////////////////////////////////////////////////////
    // calculate king moves, no castle moves yet
    const bitb ownKing = (wtm ? b.wKing : b.bKing);
    const int ownKingIndex = BSF(ownKing);
    // all squares that the king attacks
    const bitb kingAtks = kingAttacks[ownKingIndex];
    // king can push to all squares that are not occupied and not in danger
    bitb kingPushes = kingAtks & ~kingDanger & ~b.occ;
    // king can capture on all squares occupied by enemies and not in danger
    bitb kingCaptures = kingAtks & enemies & ~kingDanger;
    
    // add king moves to movelist
    while (kingCaptures) { // captures
	const int to = BSF(kingCaptures);
	movelist.push_back(Move(ownKingIndex, to, true, false, false,
						  false, false, ' '));
	kingCaptures &= kingCaptures - 1;
    }
    while (kingPushes) { // quiet moves
	const int to = BSF(kingPushes);
	movelist.push_back(Move(ownKingIndex, to, false, false, false,
						  false, false, ' '));
	kingPushes &= kingPushes - 1;
    }

    ///////////////////////////////////////////////////////
    //                       checks                      //
    ///////////////////////////////////////////////////////
    // Define a 'pushMask', all the square where we may move a piece without
    // a capture and a 'captureMask', all the square where we may capture on.
    // If we are not in check both mask consist of all squares on the board.
    bitb pushMask = 0xffffffffffffffffULL;
    bitb captureMask = 0xffffffffffffffffULL;

    // enemy pieces by type
    const bitb enemyPawns = (wtm ? b.bPawns : b.wPawns);
    const bitb enemyKnights = (wtm ? b.bKnights : b.wKnights);
    const bitb enemyBishops = (wtm ? b.bBishops : b.wBishops);
    const bitb enemyRooks = (wtm ? b.bRooks : b.wRooks);
    const bitb enemyQueens = (wtm ? b.bQueens : b.wQueens);
    const bitb enemyRooklikeSlider = enemyRooks | enemyQueens;
    const bitb enemyBishoplikeSlider = enemyBishops | enemyQueens;

    // test if we are in check at all by looking at the king danger
    if (ownKing & kingDanger) {
	// We are in check... If it's double check we can immediatly leaf the
	// move generation, as double check only allows for non-castling king
	// moves. If it's a single check we have the options to capture the
	// checking piece (handled by the 'captureMask') or to block the check
	// in the case of a slider (handled by the 'pushMask').
	const bitb pawnChecker = enemyPawns & pawnAttacks(wtm, ownKing);
	const bitb knightChecker = enemyKnights & knightAttacks[ownKingIndex];
	const bitb diagChecker
	    = enemyBishoplikeSlider & bishopAttacks(b.occ, ownKingIndex);
	const bitb lateralChecker
	    = enemyRooklikeSlider & rookAttacks(b.occ, ownKingIndex);

	// all checker
	const bitb checker = pawnChecker | knightChecker
			    | diagChecker | lateralChecker;
	const int nChecker = POP(checker); // number of pieces giving check
					   
	if (nChecker > 1) return; // double check, we are done
	// seems like we are only checked by a single piece
	captureMask = checker; // we may always capture the checking piece
	// 'pushMask' is empty unless it's a sliding piece giving check
	pushMask = 0x0ULL;
	if (diagChecker) { // bishop or queen
	    const bitb kingRays = bishopAttacks(b.occ, ownKingIndex);
	    const bitb checkerRays = bishopAttacks(b.occ, BSF(diagChecker));
	    pushMask = kingRays & checkerRays;
	} else if (lateralChecker) { // rook or queen
	    const bitb kingRays = rookAttacks(b.occ, ownKingIndex);
	    const bitb checkerRays = rookAttacks(b.occ, BSF(lateralChecker));
	    pushMask = kingRays & checkerRays;
	}
    } // end of 'inCheck' section
    
    // adjust capture mask to only include actual enemy pieces
    captureMask &= enemies;
    // adjust push mask to only include empty squares
    pushMask &= ~b.occ;

    ///////////////////////////////////////////////////////
    //                  pinned pieces                    //
    ///////////////////////////////////////////////////////
    // detect all pinned pieces... TODO
    bitb latPinned = detectLaterallyPinnedPieces(gs);
    bitb diaPinned = detectDiagonallyPinnedPieces(gs);
    bitb pinned = latPinned | diaPinned;

    ///////////////////////////////////////////////////////
    //                       rooks                       //
    ///////////////////////////////////////////////////////
    bitb ownRooks = (b.wRooks | b.bRooks) & ~enemies;
    ownRooks &= ~pinned; // TODO this needs modification
    while (ownRooks) {
	const int from = BSF(ownRooks);
	ownRooks &= ownRooks - 1;
	const bitb atks = rookAttacks(b.occ, from);
	bitb pushes = atks & pushMask;
	bitb captures = atks & captureMask;
	// add moves to movelist, captures first
	while (captures) {
	    const int to = BSF(captures);
	    captures &= captures - 1;
	    movelist.push_back(Move(from, to, true, false, false,
						false, false, ' '));
	}
	while (pushes) {
	    const int to = BSF(pushes);
	    pushes &= pushes - 1;
	    movelist.push_back(Move(from, to, false, false, false,
						false, false, ' '));
	}
    }
} // end of generateLegalMoves

/******************************************************************************
 * Returns a bitmap of all pieces in an absolute pin from a rooklike enemy
 * slider. (moving side only)
 *
 * Overview:
 * 	- generate rooklike movement of the king on an empty board and check
 * 		if there are any enemy rooks or queens at all
 * 	- if so get their attack-square using the true board-occupancy
 * 	- AND the atk-square of king and potential pinner
 * 	- if this AND-mask coincides with a friendly piece its pinned D:
 */
bitb detectLaterallyPinnedPieces(const GameState& gs) {
    bitb pinned = 0x0ULL;

    const int ownKingIndex
		    = BSF(gs.whiteToMove ? gs.board.wKing : gs.board.bKing);

    // check if there can even be laterally pinned pieces
    const bitb kingOnEmptyBoard = rookAttacks(0x0ULL, ownKingIndex);

    return pinned;
}

/******************************************************************************
 * (similar to 'detectLaterallyPinnedPieces')
 *
 * Returns a bitmap of all pieces in an absolute pin from a bishoplike enemy
 * slider. (moving side only)
 *
 * Overview:
 * 	- generate bishoplike movement of the king on an empty board and check
 * 		if there are any enemy bishops or queens at all
 * 	- if so get their attack-square using the true board-occupancy
 * 	- AND the atk-square of king and potential pinner
 * 	- if this AND-mask coincides with a friendly piece its pinned D:
 */
bitb detectDiagonallyPinnedPieces(const GameState& gs) {
    bitb pinned = 0x0ULL;

    // check if there can even be diagonally pinned pieces

    return pinned;
}

/******************************************************************************
 * Function that calculates all square attacked by the opponent.
 */
bitb generateKingDanger(const GameState& gs) {
    // reference to board
    const Board& b = gs.board;

    // occupation without own king
    const bitb modocc = b.occ ^ (gs.whiteToMove ? b.wKing : b.bKing);

    // enemy pieces by type
    bitb enemyPawns = (gs.whiteToMove ? b.bPawns : b.wPawns);
    bitb enemyKnights = (gs.whiteToMove ? b.bKnights : b.wKnights);
    bitb enemyBishops = (gs.whiteToMove ? b.bBishops : b.wBishops);
    bitb enemyRooks = (gs.whiteToMove ? b.bRooks : b.wRooks);
    bitb enemyQueens = (gs.whiteToMove ? b.bQueens : b.wQueens);
    bitb enemyKing = (gs.whiteToMove ? b.bKing : b.wKing);
    bitb enemyRooklikeSlider = enemyRooks | enemyQueens;
    bitb enemyBishoplikeSlider = enemyBishops | enemyQueens;

    // generate king danger
    bitb kingDanger = kingAttacks[BSF(enemyKing)];
    while (enemyKnights) {
	kingDanger |= knightAttacks[BSF(enemyKnights)];
	enemyKnights &= enemyKnights - 1;
    }
    while (enemyRooklikeSlider) {
	kingDanger |= rookAttacks(modocc, BSF(enemyRooklikeSlider));
	enemyRooklikeSlider &= enemyRooklikeSlider - 1;
    }
    while (enemyBishoplikeSlider) {
	kingDanger |= bishopAttacks(modocc, BSF(enemyBishoplikeSlider));
	enemyBishoplikeSlider &= enemyBishoplikeSlider - 1;
    }
    kingDanger |= pawnAttacks(!gs.whiteToMove, enemyPawns);

    return kingDanger;
}

bitb pawnAttacks(const bool whiteToMove, const bitb pawns) {
    constexpr bitb aFile = 0x8080808080808080ULL;
    constexpr bitb hFile = 0x101010101010101ULL;

    if (whiteToMove) {
	return ((pawns & ~hFile) << 7) | ((pawns & ~aFile) << 9);
    } else {
	return ((pawns & ~hFile) >> 9) | ((pawns & ~aFile) >> 7);
    }
}

bitb rookAttacks(const bitb occ, const int index) {
    bitb atks = 0ULL;
    bitb atkray, blocker;
    int sq; 

    // north
    atkray = nRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = BSF(blocker);
	atkray ^= nRays[sq];
    }
    atks |= atkray;

    // east
    atkray = eRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = BSB(blocker);
	atkray ^= eRays[sq];
    }
    atks |= atkray;

    // south
    atkray = sRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = BSB(blocker);
	atkray ^= sRays[sq];
    }
    atks |= atkray;

    // west
    atkray = wRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = BSF(blocker);
	atkray ^= wRays[sq];
    }
    atks |= atkray;

    return atks;
}

bitb bishopAttacks(const bitb occ, const int index) {
    bitb atks = 0ULL;
    bitb atkray, blocker;
    int sq; 

    // north east
    atkray = neRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = __builtin_ffsll(blocker) - 1;
	atkray ^= neRays[sq];
    }
    atks |= atkray;

    // north west
    atkray = nwRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = __builtin_ffsll(blocker) - 1;
	atkray ^= nwRays[sq];
    }
    atks |= atkray;

    // south west
    atkray = swRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = 63 - __builtin_clzll(blocker);
	atkray ^= swRays[sq];
    }
    atks |= atkray;

    // south east
    atkray = seRays[index];
    blocker = atkray & occ;
    if (blocker) {
	sq = 63 - __builtin_clzll(blocker);
	atkray ^= seRays[sq];
    }
    atks |= atkray;

    return atks;
}
