#include "movegeneration.hpp"
#include "attacks.hpp"
#include "util.hpp"

namespace azalea {

void generate_legal_moves(Board& board, Movelist& mlist) {
generate_pseudolegal_moves(board, mlist);
    for (size_t i=0; i<mlist.size(); i++) {
        if (not board.is_legal(mlist.moves[i])) mlist.remove_at(i--);
    }
}

void generate_pseudolegal_moves(Board& board, Movelist& mlist) {
    const Bitboard& us = board.white_to_move ? board.white_pieces : board.black_pieces;
    const Bitboard& them = board.white_to_move ? board.black_pieces : board.white_pieces;
    const Bitboard occ = board.white_pieces | board.black_pieces;
    const Bitboard empty = ~occ;

    // -------------------------- pawns --------------------------
    Bitboard my_pawns = us & board.pawns;

    // single pawn push
    Bitboard single_push = board.white_to_move ? my_pawns << 8 : my_pawns >> 8;
    single_push &= empty;
    while (single_push != 0) {
        const int sq = pop_lsb(single_push);
        if (sq < 8 or sq > 55) {
            mlist.push(Move(KNIGHT_PROMOTION, board.white_to_move ? sq-8 : sq+8, sq));
            mlist.push(Move(BISHOP_PROMOTION, board.white_to_move ? sq-8 : sq+8, sq));
            mlist.push(Move(ROOK_PROMOTION, board.white_to_move ? sq-8 : sq+8, sq));
            mlist.push(Move(QUEEN_PROMOTION, board.white_to_move ? sq-8 : sq+8, sq));
        } else 
            mlist.push(Move(board.white_to_move ? sq-8 : sq+8, sq));
    }

    // double push
    constexpr Bitboard rank2 = 0xFF00ULL; // 1111 1111 0000 0000
    constexpr Bitboard rank7 = 0xFF000000000000ULL; // 1111 1111 0000 0000 ...
    Bitboard double_push = my_pawns & (board.white_to_move ? rank2 : rank7);
    double_push = board.white_to_move ? double_push << 8 : double_push >> 8;
    double_push &= empty;
    double_push = board.white_to_move ? double_push << 8 : double_push >> 8;
    double_push &= empty;
    while (double_push != 0) {
        const int sq = pop_lsb(double_push);
        mlist.push(Move(DOUBLE_PAWN_PUSH, board.white_to_move ? sq-16 : sq+16, sq));
    }

    // captures
    while (my_pawns != 0) {
        const int from = pop_lsb(my_pawns);
        Bitboard p_atks = attacks::pawn(board.white_to_move, from);
        if (board.en_passant_target() != -1) {
            Bitboard ep_cap = p_atks & (1ULL << board.en_passant_target());
            if (ep_cap != 0) mlist.push(Move(EN_PASSANT, from, board.en_passant_target()));
        }
        p_atks &= them;
        while (p_atks != 0) {
            const int to = pop_lsb(p_atks);
            if (to < 8 or to > 55) {
                mlist.push(Move(CAPTURE | KNIGHT_PROMOTION, from, to));
                mlist.push(Move(CAPTURE | BISHOP_PROMOTION, from, to));
                mlist.push(Move(CAPTURE | ROOK_PROMOTION,   from, to));
                mlist.push(Move(CAPTURE | QUEEN_PROMOTION,  from, to));
            } else 
                mlist.push(Move(CAPTURE, from, to));
        }
    }


    // -------------------------- knights --------------------------
    Bitboard my_knights = us & board.knights;
    while (my_knights != 0) {
        const int from = pop_lsb(my_knights);
        Bitboard n_atks = attacks::knight(from);
        Bitboard n_caps = n_atks & them;
        Bitboard n_quiet = n_atks & empty;
        while (n_caps != 0) {
            const int to = pop_lsb(n_caps);
            mlist.push(Move(CAPTURE, from, to));
        }
        while (n_quiet != 0) {
            const int to = pop_lsb(n_quiet);
            mlist.push(Move(from, to));
        }
    }
    

    // -------------------------- bishops --------------------------
    Bitboard my_bishops = us & board.bishops;
    while (my_bishops != 0) {
        const int from = pop_lsb(my_bishops);
        Bitboard b_atks = attacks::bishop(from, occ);
        Bitboard b_caps = b_atks & them;
        Bitboard b_quiet = b_atks & empty;
        while (b_caps != 0) {
            const int to = pop_lsb(b_caps);
            mlist.push(Move(CAPTURE, from, to));
        }
        while (b_quiet != 0) {
            const int to = pop_lsb(b_quiet);
            mlist.push(Move(from, to));
        }
    }
    

    // -------------------------- rooks --------------------------
    Bitboard my_rooks = us & board.rooks;
    while (my_rooks != 0) {
        const int from = pop_lsb(my_rooks);
        Bitboard r_atks = attacks::rook(from, occ);
        Bitboard r_caps = r_atks & them;
        Bitboard r_quiet = r_atks & empty;
        while (r_caps != 0) {
            const int to = pop_lsb(r_caps);
            mlist.push(Move(CAPTURE, from, to));
        }
        while (r_quiet != 0) {
            const int to = pop_lsb(r_quiet);
            mlist.push(Move(from, to));
        }
    }


    // -------------------------- queens --------------------------
    Bitboard my_queens = us & board.queens;
    while (my_queens != 0) {
        const int from = pop_lsb(my_queens);
        Bitboard q_atks = attacks::queen(from, occ);
        Bitboard q_caps = q_atks & them;
        Bitboard q_quiet = q_atks & empty;
        while (q_caps != 0) {
            const int to = pop_lsb(q_caps);
            mlist.push(Move(CAPTURE, from, to));
        }
        while (q_quiet != 0) {
            const int to = pop_lsb(q_quiet);
            mlist.push(Move(from, to));
        }
    }


    // -------------------------- king --------------------------
    Bitboard my_king = us & board.kings;
    const int from = pop_lsb(my_king);
    Bitboard k_atks = attacks::king(from);
    Bitboard k_caps = k_atks & them;
    Bitboard k_quiet = k_atks & empty;
    while (k_caps != 0) {
        const int to = pop_lsb(k_caps);
        mlist.push(Move(CAPTURE, from, to));
    }
    while (k_quiet != 0) {
        const int to = pop_lsb(k_quiet);
        mlist.push(Move(from, to));
    }

    // castle
    if (board.white_to_move) {
        if ((board.castle_rights() & WHITE_SHORT) != 0)
            if ((occ & 0x60ULL) == 0)
                mlist.push(Move(KING_SIDE_CASTLE, E1, G1));
        if ((board.castle_rights() & WHITE_LONG ) != 0)
            if ((occ & 0x0EULL) == 0)
                mlist.push(Move(QUEEN_SIDE_CASTLE, E1, C1));
    } else { // black to move
        if ((board.castle_rights() & BLACK_SHORT) != 0)
            if ((occ & (0x60ULL << 56)) == 0)
                mlist.push(Move(KING_SIDE_CASTLE, E8, G8));
        if ((board.castle_rights() & BLACK_LONG ) != 0)
            if ((occ & (0x0EULL << 56)) == 0)
                mlist.push(Move(QUEEN_SIDE_CASTLE, E8, C8));
    }
}

} // namespace azalea
