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
void generateLegalMoves(const GameState& gs, std::vector<Move>& movelist,
							    bool& inCheck) {
    inCheck = false;

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
	inCheck = true;
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
    // detect all pinned pieces, differentiate between horizontal/vertical
    // pins and diagonal/antidiagonal pins
    bitb latPinned = detectLaterallyPinnedPieces(gs);
    bitb diaPinned = detectDiagonallyPinnedPieces(gs);
    bitb pinned = latPinned | diaPinned;

    ///////////////////////////////////////////////////////
    //                      queens                       //
    ///////////////////////////////////////////////////////
    // queens are just handled together with rooks and bishops
    const bitb ownQueens = (b.wQueens | b.bQueens) & ~enemies;

    ///////////////////////////////////////////////////////
    //                       rooks                       //
    ///////////////////////////////////////////////////////
    bitb ownRooks = (b.wRooks | b.bRooks) & ~enemies;
    ownRooks |= ownQueens;
    // pinned rooks... diagonally pinned rooks can never move...
    // laterally rooks need special care
    bitb pinnedRooks = ownRooks & latPinned;
    ownRooks &= ~pinned; // easy to handle
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
    // for pinned rooks the rooklike attacks of the king on an empty board are
    // calculated, as well as the rooks true attacks. Both are combined with a
    // bitwise AND to obtain the final square the piece can move to.
    while (pinnedRooks) {
	const bitb kingRays = rookAttacks(0x0ULL, ownKingIndex); // this could
								 // be move out
								 // of the loop
	const int from = BSF(pinnedRooks);
	pinnedRooks &= pinnedRooks - 1;
	const bitb pieceRays = rookAttacks(b.occ, from);
	const bitb atks = pieceRays & kingRays;
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

    ///////////////////////////////////////////////////////
    //                      bishops                      //
    ///////////////////////////////////////////////////////
    bitb ownBishops = (b.wBishops | b.bBishops) & ~enemies;
    ownBishops |= ownQueens;
    // pinned bishops... laterally pinned bishops can never move...
    // diagonally pinned bishops need special care
    bitb pinnedBishops = ownBishops & diaPinned;
    ownBishops &= ~pinned; // easy to handle
    while (ownBishops) {
	const int from = BSF(ownBishops);
	ownBishops &= ownBishops - 1;
	const bitb atks = bishopAttacks(b.occ, from);
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
    // for pinned bishops the bishoplike attacks of the king on an empty board
    // are calculated, as well as the bishops true attacks. Both are combined
    // with a bitwise AND to obtain the final square the piece can move to.
    while (pinnedBishops) {
	const bitb kingRays = bishopAttacks(0x0ULL, ownKingIndex); //this could
								 // be move out
								 // of the loop
	const int from = BSF(pinnedBishops);
	pinnedBishops &= pinnedBishops - 1;
	const bitb pieceRays = bishopAttacks(b.occ, from);
	const bitb atks = pieceRays & kingRays;
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

    ///////////////////////////////////////////////////////
    //                      knights                      //
    ///////////////////////////////////////////////////////
    bitb ownKnights = (b.wKnights | b.bKnights) & ~enemies;
    // pinned knights can never move
    ownKnights &= ~pinned;
    while (ownKnights) {
	const int from = BSF(ownKnights);
	ownKnights &= ownKnights - 1;
	const bitb atks = knightAttacks[from];
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

    ///////////////////////////////////////////////////////
    //                       pawns                       //
    ///////////////////////////////////////////////////////
    // TODO: The pawn section is very shitty, should be fixed at some point...
    // pushes
    constexpr bitb rank2 = 0xff00ULL;
    constexpr bitb rank7 = 0xff000000000000ULL;
    // white pawns
    if (gs.whiteToMove) {
	bitb ownPawns = b.wPawns;
	// diagonally pinned pawns can never push
	ownPawns &= ~diaPinned;
	// take laterally pinned pawns for later
	bitb pinnedPawns = ownPawns & latPinned;
	// let only non-pinned pawns remain
	ownPawns &= ~latPinned;
	bitb pushes = ownPawns << 8; // push
	bitb freePushes = pushes & ~b.occ; // free?
	freePushes &= pushMask; // on pushMask?
	while (freePushes) {
	    const int to = BSF(freePushes);
	    freePushes &= freePushes - 1;
	    if (to>55) { // promotion!
		movelist.push_back(Move(to-8, to, false, false, false,
						    false, true, 'q'));
		movelist.push_back(Move(to-8, to, false, false, false,
						    false, true, 'n'));
		movelist.push_back(Move(to-8, to, false, false, false,
						    false, true, 'r'));
		movelist.push_back(Move(to-8, to, false, false, false,
						    false, true, 'b'));

	    } else {
		movelist.push_back(Move(to-8, to, false, false, false,
						    false, false, ' '));
	    }
	}

	// double pawn pushes...
	// push the pawns from the 2nd rank once, then check if this push was
	// not occupied. If so push again, check if free and check if in push-
	// mask
	bitb freeDoublePushes = (ownPawns & rank2) << 8; // first push
	freeDoublePushes &= ~b.occ; // free?
	freeDoublePushes = freeDoublePushes << 8; // second push
	freeDoublePushes &= ~b.occ; // free?
	freeDoublePushes &= pushMask; // second push on pushMask?
	while (freeDoublePushes) {
	    const int to = BSF(freeDoublePushes);
	    freeDoublePushes &= freeDoublePushes - 1;
	    movelist.push_back(Move(to-16, to, false, true, false,
						false, false, ' '));
	}

	// pinned pawns
	// so... these are either vertically or horizontally pinned...
	// they can never promote.
	// they can not push if the pin is horizontal
	while (pinnedPawns) {
	    const int from = BSF(pinnedPawns);
	    pinnedPawns &= pinnedPawns - 1;
	    // check for horizontal pin
	    if (from/8 == ownKingIndex/8) continue;
	    // so this pawn in vertically pinned, this means it can just move
	    // normally, respecting the push mask
	    bitb pawnMask = (1ULL << from);
	    bitb singlePush = (pawnMask << 8) & ~b.occ & pushMask;
	    if (singlePush) {
		movelist.push_back(Move(from, from+8, false, false, false,
						    false, false, ' '));
	    }
	    if (from < 16) // pawn on starting rank
	    if ((pawnMask << 8) & ~b.occ) { // single push is free
		const int to = from + 16;
		if ((1ULL << to) & ~b.occ & pushMask) // double push allowed
		movelist.push_back(Move(to-16, to, false, true, false,
						    false, false, ' '));
	    }
	}

    // black pawns
    } else {
	bitb ownPawns = b.bPawns;
	// diagonally pinned pawns can never push
	ownPawns &= ~diaPinned;
	// take laterally pinned pawns for later
	bitb pinnedPawns = ownPawns & latPinned;
	// let only non-pinned pawns remain
	ownPawns &= ~latPinned;
	bitb pushes = ownPawns >> 8; // push
	bitb freePushes = pushes & ~b.occ; // free?
	freePushes &= pushMask; // on pushMask?
	while (freePushes) {
	    const int to = BSF(freePushes);
	    freePushes &= freePushes - 1;
	    if (to<8) { // promotion!
		movelist.push_back(Move(to+8, to, false, false, false,
						    false, true, 'q'));
		movelist.push_back(Move(to+8, to, false, false, false,
						    false, true, 'n'));
		movelist.push_back(Move(to+8, to, false, false, false,
						    false, true, 'r'));
		movelist.push_back(Move(to+8, to, false, false, false,
						    false, true, 'b'));

	    } else {
		movelist.push_back(Move(to+8, to, false, false, false,
						    false, false, ' '));
	    }
	}

	// double pawn pushes...
	// push the pawns from the 7th rank once, then check if this push was
	// not occupied. If so push again, check if free and check if in push-
	// mask
	bitb freeDoublePushes = (ownPawns & rank7) >> 8; // first push
	freeDoublePushes &= ~b.occ; // free?
	freeDoublePushes = freeDoublePushes >> 8; // second push
	freeDoublePushes &= ~b.occ; // free?
	freeDoublePushes &= pushMask; // second push on pushMask?
	while (freeDoublePushes) {
	    const int to = BSF(freeDoublePushes);
	    freeDoublePushes &= freeDoublePushes - 1;
	    movelist.push_back(Move(to+16, to, false, true, false,
						false, false, ' '));
	}

	// pinned pawns
	// so... these are either vertically or horizontally pinned...
	// they can never promote.
	// they can not push if the pin is horizontal
	while (pinnedPawns) {
	    const int from = BSF(pinnedPawns);
	    pinnedPawns &= pinnedPawns - 1;
	    // check for horizontal pin
	    if (from/8 == ownKingIndex/8) continue;
	    // so this pawn in vertically pinned, this means it can just move
	    // normally, respecting the push mask
	    bitb pawnMask = (1ULL << from);
	    bitb singlePush = (pawnMask >> 8) & ~b.occ & pushMask;
	    if (singlePush) {
		movelist.push_back(Move(from, from-8, false, false, false,
						    false, false, ' '));
	    }
	    if (from > 47) // pawn on starting rank
	    if ((pawnMask >> 8) & ~b.occ) { // single push is free
		const int to = from - 16;
		if ((1ULL << to) & ~b.occ & pushMask) // double push allowed
		movelist.push_back(Move(to+16, to, false, true, false,
						    false, false, ' '));
	    }
	}
    }

    // pawn captures, oh boi...
    constexpr bitb aFile = 0x8080808080808080ULL;
    constexpr bitb hFile = 0x101010101010101ULL;

    // white pawns (captures)
    if (gs.whiteToMove) {
	bitb ownPawns = b.wPawns & ~pinned;
	bitb pinnedPawns = b.wPawns & diaPinned;
	// capture to the left (nw, <<9) so no pawns from the a file
	bitb leftAtks = (ownPawns & ~aFile) << 9;
	leftAtks &= captureMask;
	while (leftAtks) {
	    const int to = BSF(leftAtks);
	    leftAtks &= leftAtks - 1;
	    if (to>55) { // promotion!
		movelist.push_back(Move(to-9, to, true, false, false,
						    false, true, 'q'));
		movelist.push_back(Move(to-9, to, true, false, false,
						    false, true, 'n'));
		movelist.push_back(Move(to-9, to, true, false, false,
						    false, true, 'r'));
		movelist.push_back(Move(to-9, to, true, false, false,
						    false, true, 'b'));
	    } else {
		movelist.push_back(Move(to-9, to, true, false, false,
						    false, false, ' '));
	    }
	}
	// capture to the right (ne, <<7) so no pawns from the h file
	bitb rightAtks = (ownPawns & ~hFile) << 7;
	rightAtks &= captureMask;
	while (rightAtks) {
	    const int to = BSF(rightAtks);
	    rightAtks &= rightAtks - 1;
	    if (to>55) { // promotion!
		movelist.push_back(Move(to-7, to, true, false, false,
						    false, true, 'q'));
		movelist.push_back(Move(to-7, to, true, false, false,
						    false, true, 'n'));
		movelist.push_back(Move(to-7, to, true, false, false,
						    false, true, 'r'));
		movelist.push_back(Move(to-7, to, true, false, false,
						    false, true, 'b'));
	    } else {
		movelist.push_back(Move(to-7, to, true, false, false,
						    false, false, ' '));
	    }
	}
	// now handle the pinned pawns
	while (pinnedPawns) {
	    const int from = BSF(pinnedPawns);
	    pinnedPawns &= pinnedPawns - 1;
	    // pinned along a diagonal
	    if (diags[from] == diags[ownKingIndex]) {
		// here we can capture to the left, so +9
		const int to = from + 9;
		if ((1ULL<<to) & captureMask) {
		    if (to>55) { // promotion
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'q'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'n'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'r'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'b'));
		    } else {
			movelist.push_back(Move(from, to, true, false, false,
							false, false, ' '));
		    }
		}
	    // pinned along a antidiagonal
	    } else if (antidiags[from] == antidiags[ownKingIndex]) {
		// here we can capture to the right, so +7
		const int to = from + 7;
		if ((1ULL<<to) & captureMask) {
		    if (to>55) { // promotion
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'q'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'n'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'r'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'b'));
		    } else {
			movelist.push_back(Move(from, to, true, false, false,
							false, false, ' '));
		    }
		}
	    }
	}

    // black pawns (captures)
    } else {
	bitb ownPawns = b.bPawns & ~pinned;
	bitb pinnedPawns = b.bPawns & diaPinned;
	// capture to the left (sw, >>7) so no pawns from the a file
	bitb leftAtks = (ownPawns & ~aFile) >> 7;
	leftAtks &= captureMask;
	while (leftAtks) {
	    const int to = BSF(leftAtks);
	    leftAtks &= leftAtks - 1;
	    if (to<8) { // promotion!
		movelist.push_back(Move(to+7, to, true, false, false,
						    false, true, 'q'));
		movelist.push_back(Move(to+7, to, true, false, false,
						    false, true, 'n'));
		movelist.push_back(Move(to+7, to, true, false, false,
						    false, true, 'r'));
		movelist.push_back(Move(to+7, to, true, false, false,
						    false, true, 'b'));
	    } else {
		movelist.push_back(Move(to+7, to, true, false, false,
						    false, false, ' '));
	    }
	}
	// capture to the right (ne, >>9) so no pawns from the h file
	bitb rightAtks = (ownPawns & ~hFile) >> 9;
	rightAtks &= captureMask;
	while (rightAtks) {
	    const int to = BSF(rightAtks);
	    rightAtks &= rightAtks - 1;
	    if (to<8) { // promotion!
		movelist.push_back(Move(to+9, to, true, false, false,
						    false, true, 'q'));
		movelist.push_back(Move(to+9, to, true, false, false,
						    false, true, 'n'));
		movelist.push_back(Move(to+9, to, true, false, false,
						    false, true, 'r'));
		movelist.push_back(Move(to+9, to, true, false, false,
						    false, true, 'b'));
	    } else {
		movelist.push_back(Move(to+9, to, true, false, false,
						    false, false, ' '));
	    }
	}
	// now handle the pinned pawns
	while (pinnedPawns) {
	    const int from = BSF(pinnedPawns);
	    pinnedPawns &= pinnedPawns - 1;
	    // pinned along a diagonal
	    if (diags[from] == diags[ownKingIndex]) {
		// here we can capture to the right, so -9
		const int to = from - 9;
		if ((1ULL<<to) & captureMask) {
		    if (to>55) { // promotion
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'q'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'n'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'r'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'b'));
		    } else {
			movelist.push_back(Move(from, to, true, false, false,
							false, false, ' '));
		    }
		}
	    // pinned along a antidiagonal
	    } else if (antidiags[from] == antidiags[ownKingIndex]) {
		// here we can capture to the left, so -7
		const int to = from - 7;
		if ((1ULL<<to) & captureMask) {
		    if (to>55) { // promotion
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'q'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'n'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'r'));
			movelist.push_back(Move(from, to, true, false, false,
							false, true, 'b'));
		    } else {
			movelist.push_back(Move(from, to, true, false, false,
							false, false, ' '));
		    }
		}
	    }
	}
    }

    // all en passant captures are handled here
    if (gs.epTarget != -1) {
	// check if the pawn in question may be captures at all...
	// If the enemy pawn is pinned to our king diagonally we may never
	// ep cature it. (by the definition used here it can not be laterally
	// pinned, this is not completely true but will be handled later)
	const bitb pawnToCaptureIndex
			= (gs.whiteToMove ? gs.epTarget-8 : gs.epTarget+8);
	if ((1ULL<<pawnToCaptureIndex) & diaPinned) {
	    // cannot be captured
	    goto noEP;
	}
	// if pawn is not in captureMask also done
	if (not ((1ULL<<pawnToCaptureIndex) & captureMask)) {
	    // cannot be captured
	    goto noEP;
	}

	const bitb potentialPawns
			= pawnAttacks(!gs.whiteToMove, (1ULL<<gs.epTarget));
	bitb ownPawns = (gs.whiteToMove ? b.wPawns : b.bPawns);
	ownPawns &= potentialPawns;
	// diagonally pinned pawns for later
	bitb diaPinnedPawns = ownPawns & diaPinned;
	// laterally pinned pawns can never capture
	ownPawns &= ~pinned;
	while(ownPawns) {
	    const int from = BSF(ownPawns);
	    ownPawns &= ownPawns - 1;
	    // rook  pawn  pawn  king arragement
	    // test if its even possible
	    const bitb specialRank
	    		= (gs.whiteToMove ? 0xff00000000ULL : 0xff000000ULL);
	    if (ownKing & specialRank) { // is my king even on the right rank?
	        bitb potentialRooks = enemyRooklikeSlider & specialRank;
	        if (potentialRooks) { // also a rook/queen?
	    	   // mask with only both pawns in question
	    	   const bitb bothPawns
		   		= (1ULL<<pawnToCaptureIndex) | (1ULL<<from);
	    	   while(potentialRooks) {
	    	       const int index = BSF(potentialRooks);
	    	       potentialRooks &= potentialRooks - 1;
		       bitb overlap = (wRays[ownKingIndex] & eRays[index])
					| (eRays[ownKingIndex] & wRays[index]);
		       overlap &= b.occ;
		       if (overlap == bothPawns) goto noEP;
	    	   }
	        }
	    }
	    movelist.push_back(Move(from, gs.epTarget, true, false, true,
							false, false, ' '));
	}
	// dia pinned pawns here
	while(diaPinnedPawns) {
	    const int from = BSF(diaPinnedPawns);
	    diaPinnedPawns &= diaPinnedPawns - 1;
	    // same diagonal
	    if (diags[from] == diags[ownKingIndex]
		and diags[from] == diags[gs.epTarget]) {
		     movelist.push_back(
			     Move(from, gs.epTarget, true, false, true,
						    false, false, ' '));
	    // same antidiagonal
	    } else if (antidiags[from] == antidiags[ownKingIndex]
		    and antidiags[from] == antidiags[gs.epTarget]) {
		     movelist.push_back(
			     Move(from, gs.epTarget, true, false, true,
						    false, false, ' '));
	    }
	}
    }
    noEP:

    ///////////////////////////////////////////////////////
    //                      castling                     //
    ///////////////////////////////////////////////////////
    if (wtm) { // white
	// white kingside
	if (gs.whiteShort) {
    	    constexpr bitb wsOccMask = 0x6ULL;
    	    constexpr bitb wsDangerMask = 0xeULL;
    	    // are f1 and g1 free?
    	    const bool isFree = (wsOccMask & ~b.occ) == wsOccMask;
    	    // are e1, f1 and g1 safe?
	    const bool isSafe = (wsDangerMask & ~kingDanger) == wsDangerMask;
	    // only castle if free and safe
	    if (isFree and isSafe) 
		movelist.push_back(Move(ownKingIndex, 1, false, false, false,
							true, false, ' '));
    	}
	// white queenside
	if (gs.whiteLong) {
    	    constexpr bitb wlOccMask = 0x70ULL;
    	    constexpr bitb wlDangerMask = 0x38ULL;
    	    // are b1, c1 and d1 free?
    	    const bool isFree = (wlOccMask & ~b.occ) == wlOccMask;
    	    // are b1, c1, d1 and e1 safe?
	    const bool isSafe = (wlDangerMask & ~kingDanger) == wlDangerMask;
	    // only castle if free and safe
	    if (isFree and isSafe) 
		movelist.push_back(Move(ownKingIndex, 5, false, false, false,
							true, false, ' '));
    	}
    } else { // black
	// black kingside
	if (gs.blackShort) {
    	    constexpr bitb bsOccMask = 0x600000000000000ULL;
    	    constexpr bitb bsDangerMask = 0xe00000000000000ULL;
    	    // are f8 and g8 free?
    	    const bool isFree = (bsOccMask & ~b.occ) == bsOccMask;
    	    // are e8, f8 and g8 safe?
	    const bool isSafe = (bsDangerMask & ~kingDanger) == bsDangerMask;
	    // only castle if free and safe
	    if (isFree and isSafe) 
		movelist.push_back(Move(ownKingIndex, 57, false, false, false,
							true, false, ' '));
    	}
	// black queenside
	if (gs.blackLong) {
    	    constexpr bitb blOccMask = 0x7000000000000000ULL;
    	    constexpr bitb blDangerMask = 0x3800000000000000ULL;
    	    // are b8, c8 and d8 free?
    	    const bool isFree = (blOccMask & ~b.occ) == blOccMask;
    	    // are c8, d8 and e8 safe?
	    const bool isSafe = (blDangerMask & ~kingDanger) == blDangerMask;
	    // only castle if free and safe
	    if (isFree and isSafe) 
		movelist.push_back(Move(ownKingIndex, 61, false, false, false,
							true, false, ' '));
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

    // reference to board
    const Board& b = gs.board;

    const int ownKingIndex = BSF(gs.whiteToMove ? b.wKing : b.bKing);
    bitb potentialPinner = (gs.whiteToMove ? (b.bRooks | b.bQueens)
					   : (b.wRooks | b.wQueens));

    // check if there can even be laterally pinned pieces
    const bitb kingOnEmptyBoard = rookAttacks(0x0ULL, ownKingIndex);
    potentialPinner &= kingOnEmptyBoard;
    if (potentialPinner) {
	// where are your frens?
	const bitb kingOnTrueBoard = rookAttacks(b.occ, ownKingIndex);
	// now iterate over the potential pinner and check if there is a
	// friendly piece they pin
	while (potentialPinner) {
	    const int potentialPinnerIndex = BSF(potentialPinner);
	    potentialPinner &= potentialPinner - 1;
	    // get its attack squares
	    const bitb atks = rookAttacks(b.occ, potentialPinnerIndex);
	    // Now overlap the attack square from the kings location and the
	    // enemies location and with all friendly pieces. Whatever is left
	    // is indeed a pinned piece! D:
	    pinned |= kingOnTrueBoard & atks;
	}
    }

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

    // reference to board
    const Board& b = gs.board;

    const int ownKingIndex = BSF(gs.whiteToMove ? b.wKing : b.bKing);
    bitb potentialPinner = (gs.whiteToMove ? (b.bBishops | b.bQueens)
					   : (b.wBishops | b.wQueens));

    // check if there can even be diagonally pinned pieces
    const bitb kingOnEmptyBoard = bishopAttacks(0x0ULL, ownKingIndex);
    potentialPinner &= kingOnEmptyBoard;
    if (potentialPinner) {
	// where are your frens?
	const bitb kingOnTrueBoard = bishopAttacks(b.occ, ownKingIndex);
	// now iterate over the potential pinner and check if there is a
	// friendly piece they pin
	while (potentialPinner) {
	    const int potentialPinnerIndex = BSF(potentialPinner);
	    potentialPinner &= potentialPinner - 1;
	    // get its attack squares
	    const bitb atks = bishopAttacks(b.occ, potentialPinnerIndex);
	    // Now overlap the attack square from the kings location and the
	    // enemies location and with all friendly pieces. Whatever is left
	    // is indeed a pinned piece! D:
	    pinned |= kingOnTrueBoard & atks;
	}
    }

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
