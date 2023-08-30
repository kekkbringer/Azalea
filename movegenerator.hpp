#pragma once

#include <vector>

#include "params.hpp"
#include "board.hpp"
#include "move.hpp"

/******************************************************************************
 * Function that generates all legal moves in a given position. Returns the
 * number of legal moves and gives the moves themself on 'movelist'.
 */
[[nodiscard]]
int generateLegalMoves(const GameState& gs, Move movelist[azalea::maxMoves],
							bool& inCheck);

/******************************************************************************
 * Function that calculates all square attacked by the opponent.
 */
bitb generateKingDanger(const GameState& gs);

/******************************************************************************
 * Returns a bitmap of all pieces in an absolute pin from a rooklike enemy
 * slider. (moving side only)
 */
bitb detectLaterallyPinnedPieces(const GameState& gs);

/******************************************************************************
 * Returns a bitmap of all pieces in an absolute pin from a bishoplike enemy
 * slider. (moving side only)
 */
bitb detectDiagonallyPinnedPieces(const GameState& gs);

bitb pawnAttacks(const bool whiteToMove, const bitb pawns);
bitb rookAttacks(const bitb occ, const int index);
bitb bishopAttacks(const bitb occ, const int index);
