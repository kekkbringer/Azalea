#include "board.hpp"
#include "attacks.hpp"
#include "util.hpp"
#include "zobrist.hpp"
#include "score.hpp"
#include "eval_trace.hpp"
#include "weights.hpp"

#include <bit>
#include <iostream>
#include <sstream>

namespace azalea {

constexpr int PAWN=0, KNIGHT=1, BISHOP=2, ROOK=3, QUEEN=4, KING=5;

constexpr int promotion_index(const Move m) {
    if (m.is_knight_promotion()) return KNIGHT;
    if (m.is_bishop_promotion()) return BISHOP;
    if (m.is_rook_promotion())   return ROOK;
    return QUEEN;
}

// helper for incremental piece eval during make_move
inline constexpr std::array<std::array<Score, 64>, 12> PIECE_SQ_TABLE = []{
    std::array<std::array<Score, 64>, 12> table{};
    for (std::size_t pt=0; pt<6; ++pt) {
        // get material value, king has 0
        const Score val = (pt < 5) ? WEIGHTS[PAWN_EVAL + pt] : Score{};
        for (std::size_t sq=0; sq<64; ++sq) {
            table[pt][sq]   =   val + WEIGHTS[PST_START + pt*64 + sq];
            table[pt+6][sq] = -(val + WEIGHTS[PST_START + pt*64 + (sq^56)]);
        }
    }
    return table;
}();

/**
 * Takes a combined index indicating piece type and color as well as the square of the piece.
 */
template <bool UpdateState>
void Board::put_piece(int pc, int sq) {
    const Bitboard mask = (1ULL << sq);

    // update color and pieces bitboards
    color_bb(pc)    |= mask;
    type_bb(pc % 6) |= mask;

    const auto pc_t = static_cast<std::size_t>(pc);
    const auto sq_t = static_cast<std::size_t>(sq);

    // update hash and eval
    if constexpr (UpdateState) {
        hash() ^= z_keys.piece_square[pc_t][sq_t];
        position_history.back().score += PIECE_SQ_TABLE[pc_t][sq_t];
    }
}

/**
 * Takes a combined index indicating piece type and color as well as the square of the piece.
 */
template <bool UpdateState>
void Board::remove_piece(int pc, int sq) {
    const Bitboard mask = (1ULL << sq);

    // update color and pieces bitboards
    color_bb(pc)    &= ~mask;
    type_bb(pc % 6) &= ~mask;

    const auto pc_t = static_cast<std::size_t>(pc);
    const auto sq_t = static_cast<std::size_t>(sq);

    // update hash and eval
    if constexpr (UpdateState) {
        hash() ^= z_keys.piece_square[pc_t][sq_t];
        position_history.back().score -= PIECE_SQ_TABLE[pc_t][sq_t];
    }
}

/**
 * Takes a combined index indicating piece type and color as well as the square of the piece.
 */
template <bool UpdateState>
void Board::move_piece(int pc, int from, int to) {
    remove_piece<UpdateState>(pc, from);
    put_piece<UpdateState>(pc, to);
}


// TODO: this is the inefficient way to check if a move gives check
[[nodiscard]] bool Board::gives_check(const Move move) {
    make_move(move);
    const bool gives_check = in_check();
    unmake_move(move);
    return gives_check;
}

[[nodiscard]] bool Board::player_has_non_pawn_material() const {
    const Bitboard& us = white_to_move ? white_pieces : black_pieces;
    return ((us & ~(pawns | kings)) != 0);
}

void Board::make_null_move() {
    position_history.push_back(position_history.back());
    auto& state = position_history.back();
    if (state.en_passant_target != -1) {
        hash() ^= z_keys.en_passant[static_cast<std::size_t>(en_passant_target()%8)];
        state.en_passant_target = -1;
    }
    hash() ^= z_keys.side_to_move;
    white_to_move = not white_to_move;
    state.halfmove_clock++;
    state.pliesFromLastNull = 0;
    state.captured = PieceType::Empty;
    update_checker_pinned();
}

void Board::unmake_null_move() {
    white_to_move = not white_to_move;
    position_history.pop_back();
}

[[nodiscard]] bool Board::is_pseudolegal(const Move move) const {
    const int from = move.from();
    //const int to   = move.to();

    const Bitboard from_mask = (1ULL << from);
    //const Bitboard to_mask   = (1ULL << to);
    const Bitboard& us = white_to_move ? white_pieces : black_pieces;
    //const Bitboard& them = white_to_move ? black_pieces : white_pieces;

    // check if a piece is there
    if ((us & from_mask) == 0) return false;
    std::cout << "pseudolegality not yet implemented\n";
    return false;
}

/**
 * Returns a bitboard containing ALL pieces that are attacking a certain square.
 */
[[nodiscard]] Bitboard Board::attacking_to (const int sq, const Bitboard occ) const noexcept {
    return (attacks::pawn(false, sq) & white_pieces & pawns)
         | (attacks::pawn(true, sq)  & black_pieces & pawns)
         | (attacks::knight(sq) & knights)
         | (attacks::bishop(sq, occ) & (bishops | queens))
         | (attacks::rook(sq, occ) & (rooks | queens))
         | (attacks::king(sq) & kings);
}

/**
 * Updates the Bitboard containing all checking pieces and all pinned pieces.
 */
void Board::update_checker_pinned() {
    const Bitboard& us = white_to_move ? white_pieces : black_pieces;
    const Bitboard& them = white_to_move ? black_pieces : white_pieces;
    const Bitboard occ = white_pieces | black_pieces;
    const auto king_sq = lsb(us & kings);

    auto& cur = position_history.back();
    cur.checkers = attacking_to(king_sq, occ) & them;

    // get potential enemy pieces that could pin something to our king
    Bitboard pot_pinner = (attacks::rook(king_sq, 0)   & them & (rooks   | queens))
                        | (attacks::bishop(king_sq, 0) & them & (bishops | queens));

    // test all potential pinner
    cur.pinned = 0ULL;
    while (pot_pinner != 0) {
        const Bitboard between = attacks::between(king_sq, pop_lsb(pot_pinner)) & occ;
        if (std::has_single_bit(between) and (between & us)) cur.pinned |= between;
    }
}

[[nodiscard]] bool Board::is_repetition() const {
    const int n_hist = static_cast<int>(position_history.size());
    // only the last 'halfmove_clock' positions can even be a repetition
    int stop = std::max(0, n_hist - 1 - halfmove_clock());
    stop = std::max(stop, n_hist - 1 - position_history.back().pliesFromLastNull);
    // go in increments of 2, to use asymmetry of side_to_move
    for (int i=n_hist-3; i >= stop; i-=2)
        if (position_history[static_cast<std::size_t>(i)].hash == hash()) return true;
    return false;
}

[[nodiscard]] bool Board::is_attacked(const int sq, const bool by_white) const {
    const Bitboard& them = by_white ? white_pieces : black_pieces;
    const Bitboard occ = white_pieces | black_pieces;

    const Bitboard p_atks = attacks::pawn(not by_white, sq);
    if ((them & pawns & p_atks) != 0) return true;
    const Bitboard n_atks = attacks::knight(sq);
    if ((them & knights & n_atks) != 0) return true;
    const Bitboard b_atks = attacks::bishop(sq, occ);
    if ((them & (bishops | queens) & b_atks) != 0) return true;
    const Bitboard r_atks = attacks::rook(sq, occ);
    if ((them & (rooks | queens) & r_atks) != 0) return true;
    const Bitboard k_atks = attacks::king(sq);
    if ((them & kings & k_atks) != 0) return true;

    return false;
}

/**
 * The move in question must be pseudo legal, this is NOT CHECKED here.
 */
[[nodiscard]] bool Board::is_legal(const Move move) const {
    const Bitboard& us   = white_to_move ? white_pieces : black_pieces;
    const Bitboard& them = white_to_move ? black_pieces : white_pieces;
    const int from = move.from();
    const int to   = move.to();
    const int king_sq = lsb(us & kings);

    // special case: en-passant
    if (move.is_en_passant()) {
        const int captured_sq = white_to_move ? (to-8) : (to+8);
        // occupied squares with all relevant pieces on their squares after the ep cap
        const Bitboard occ = ((white_pieces | black_pieces)
                    ^ (1ULL << from) ^ (1ULL << captured_sq))
                    | (1ULL << to);
        const bool pinned_by_rook   = attacks::rook(king_sq, occ)   & them & (rooks   | queens);
        const bool pinned_by_bishop = attacks::bishop(king_sq, occ) & them & (bishops | queens);
        return (not pinned_by_rook) and (not pinned_by_bishop);
    }

    // special case: king move (the king can "shadow" an attacked square)
    if (from == king_sq) {
        // the target square and all squares in between must not be attacked
        if (move.is_king_side_castle()) {
            if (in_check()) return false;
            if (white_to_move) return not is_attacked(F1, false) and not is_attacked(G1, false);
            else return not is_attacked(F8, true) and not is_attacked(G8, true);
        } else if (move.is_queen_side_castle()) {
            if (in_check()) return false;
            if (white_to_move) return not is_attacked(D1, false) and not is_attacked(C1, false);
            else return not is_attacked(D8, true) and not is_attacked(C8, true);
        }
        // occ without out king
        const Bitboard occ = (white_pieces | black_pieces) ^ (1ULL << king_sq);
        return not (attacking_to(to, occ) & them);
    }

    // general case
    // check evasion
    if (in_check()) {
        Bitboard checkers = position_history.back().checkers;
        // double check -> king must move, already covered above
        if (more_than_one(checkers)) return false;
        const auto checker_sq = lsb(checkers);
        // one checker -> checker must be captured or the atk must be blocked
        const bool blocking_or_capturing
            = (1ULL << to) & (attacks::between(king_sq, checker_sq) | checkers);
        if (not blocking_or_capturing) return false;
    }

    // not in check
    // move is legal if the piece is not pinned
    // or it is pinned but moves along the line between the king and the checker
    const bool pinned = position_history.back().pinned & (1ULL << from);
    return (not pinned) or (attacks::line_through(from, king_sq) & (1ULL << to));
}

void Board::unmake_move(const Move move) {
    const PieceType victim = captured();
    position_history.pop_back();
    if (white_to_move) --fullmove_number;
    white_to_move = not white_to_move;

    const auto from = move.from();
    const auto to   = move.to();
    const int us   = white_to_move ? 0 : 6;
    const int them = 6 - us;

    // do everything in reverse order of make_move

    // castling -> move rook back
    if (move.is_king_side_castle()) {
        if (white_to_move) move_piece<false>(us + ROOK, F1, H1);
        else               move_piece<false>(us + ROOK, F8, H8);
    } else if (move.is_queen_side_castle()) {
        if (white_to_move) move_piece<false>(us + ROOK, D1, A1);
        else               move_piece<false>(us + ROOK, D8, A8);
    }

    // move piece back
    if (move.is_promotion()) {
        remove_piece<false>(us + promotion_index(move), to);
        put_piece<false>(us + PAWN, from);
    } else {
        move_piece<false>(us + static_cast<int>(piece_at_square(to)), to, from);
    }

    // finally, put caputured piece back
    if (move.is_en_passant()) {
        put_piece<false>(them + PAWN, white_to_move ? to-8 : to+8);
    } else if (victim != PieceType::Empty) {
        put_piece<false>(them + static_cast<int>(victim), to);
    }
}

void Board::make_move(const Move move) {
    position_history.push_back(position_history.back());
    position_history.back().pliesFromLastNull++;

    const auto& to   = move.to();
    const auto& from = move.from();

    const int us   = white_to_move ? 0 : 6;
    const int them = 6 - us;
    const int imoved = us   + static_cast<int>(piece_at_square(move.from()));

    // remove the captured piece first
    if (move.is_en_passant()) {
        const int cap_sq = white_to_move ? to-8 : to+8;
        remove_piece(them + PAWN, cap_sq);
        captured() = PieceType::Pawn;
    } else if (move.is_capture()) {
        const PieceType victim = piece_at_square(to);
        remove_piece(them + static_cast<int>(victim), to);
        captured() = victim;
    } else {
        captured() = PieceType::Empty;
    }

    // move the piece
    if (move.is_promotion()) {
        remove_piece(imoved, from);
        put_piece(us + promotion_index(move), to);
    } else {
        move_piece(imoved, from, to);
    }

    // castling -> rook still didnt get moved
    if (move.is_king_side_castle()) {
        if (white_to_move) move_piece(us + ROOK, H1, F1);
        else               move_piece(us + ROOK, H8, F8);
    } else if (move.is_queen_side_castle()) {
        if (white_to_move) move_piece(us + ROOK, A1, D1);
        else               move_piece(us + ROOK, A8, D8);
    }


    // castling rights
    hash() ^= z_keys.castling[castle_rights()];
    if (move.from() == E1) 
        castle_rights() &= static_cast<std::uint8_t>(~(WHITE_SHORT | WHITE_LONG));
    else if (move.from() == A1 or move.to() == A1)
        castle_rights() &= static_cast<std::uint8_t>(~WHITE_LONG);
    else if (move.from() == H1 or move.to() == H1)
        castle_rights() &= static_cast<std::uint8_t>(~WHITE_SHORT);
    if (move.from() == E8)
        castle_rights() &= static_cast<std::uint8_t>(~(BLACK_SHORT | BLACK_LONG));
    else if (move.from() == A8 or move.to() == A8)
        castle_rights() &= static_cast<std::uint8_t>(~BLACK_LONG);
    else if (move.from() == H8 or move.to() == H8)
        castle_rights() &= static_cast<std::uint8_t>(~BLACK_SHORT);
    hash() ^= z_keys.castling[castle_rights()];

    // en passant target
    if (en_passant_target() != -1)
        hash() ^= z_keys.en_passant[static_cast<std::size_t>(en_passant_target()%8)];
    en_passant_target() = -1;

    // double pawn push -> maybe set ep square
    if (move.is_double_pawn_push()) {
        const int ep_sq = (move.to() + move.from()) / 2;
        // only set en_passant target if a capture is possible, otherwise it needs
        // to be identical to the position where the double pawn push did not
        // occur on the last move
        const Bitboard capturers = attacks::pawn(white_to_move, ep_sq) & pawns & color_bb(them);
        if (capturers != 0) {
            en_passant_target() = ep_sq;
            hash() ^= z_keys.en_passant[static_cast<std::size_t>(move.to()%8)];
        }
    }

    ++halfmove_clock();
    if (move.is_capture() or imoved % 6 == PAWN) halfmove_clock() = 0;

    if (not white_to_move) ++fullmove_number;

    white_to_move = not white_to_move;
    hash() ^= z_keys.side_to_move;

    update_checker_pinned();
}

[[nodiscard]] PieceType Board::piece_at_square(const int sq) const noexcept {
    const Bitboard mask = (1ULL << sq);
    if ((mask & pawns) != 0) return PieceType::Pawn;
    if ((mask & knights) != 0) return PieceType::Knight;
    if ((mask & bishops) != 0) return PieceType::Bishop;
    if ((mask & rooks) != 0) return PieceType::Rook;
    if ((mask & queens) != 0) return PieceType::Queen;
    if ((mask & kings) != 0) return PieceType::King;
    return PieceType::Empty;
}

/**
 * prints bitboard formated as 8x8 board.
 */
void print_bitboard(const uint64_t bb) {
    for (int rank=7; rank>=0; rank--) {
        for (int file=0; file<8; file++) {
            const auto mask = 1ULL << (8*rank+file);
            if ((bb & mask) != 0) {
                std::cout << "1 ";
            } else {
                std::cout << "0 ";
            }
        }
        std::cout << "\n";
    }
}

Board::Board() {
    position_history.reserve(1024);
    position_history.emplace_back();
}


/**
 * Initializes the board from a FEN string.
 * TODO: Properly reject invalid FEN strings.
 */
Board::Board(const std::string_view& fen) {
    position_history.reserve(1024);
    position_history.emplace_back();
    std::istringstream iss(std::string{fen});
    std::string substring;

    hash() = 0ULL;

    // piece placement
    constexpr std::string_view piece_chars = "PNBRQKpnbrqk";
    iss >> substring;
    int index = 56; // a8
    int line_cap = 0;
    for (const auto& c: substring) {
        if (c >= '1' and c <= '8') {
            const int n = c - '0';
            index += n;
            line_cap += n;
        } else if (c == '/') {
            if (line_cap != 8) { std::cout << "Invalid FEN" << std::endl; return; }
            line_cap = 0;
            index -= 16;
        } else if (const auto pc = piece_chars.find(c); pc != std::string_view::npos) {
            put_piece(static_cast<int>(pc), index);
            ++index;
            ++line_cap;
        } else {
            std::cout << "Unknown symbol in FEN" << std::endl;
            return;
        }
    }


    // side to move
    iss >> substring;
    if (substring == "w") {
        white_to_move = true;
        hash() ^= z_keys.side_to_move;
    } else if (substring == "b") white_to_move = false;
    else {
        std::cout << "Invalid FEN" << std::endl;
        return;
    }


    // castling rights
    iss >> substring;
    castle_rights() = 0;
    for (const auto& c: substring) {
        if (substring == "-") break;
        switch (c) {
            case 'K': { castle_rights() |= WHITE_SHORT; continue; }
            case 'Q': { castle_rights() |= WHITE_LONG; continue; }
            case 'k': { castle_rights() |= BLACK_SHORT; continue; }
            case 'q': { castle_rights() |= BLACK_LONG; continue; }
            default: {
                         std::cout << "Unknown symbol in FEN" << std::endl;
                         return;
                     }
        }
    }
    hash() ^= z_keys.castling[castle_rights()];


    // en passant target
    iss >> substring;
    if (substring == "-") en_passant_target() = -1;
    else en_passant_target() = alg_to_int(substring);
    if (en_passant_target() != -1) {
        // side to move could (?) capture ep, so from their point of view
        const Bitboard capturers
            = attacks::pawn(not white_to_move, en_passant_target())
            & pawns & (white_to_move ? white_pieces : black_pieces);
        if (capturers == 0) en_passant_target() = -1;
    }
    if (en_passant_target() != -1)
        hash() ^= z_keys.en_passant[static_cast<std::size_t>(en_passant_target()%8)];


    // halfmove clock
    iss >> substring;
    halfmove_clock() = std::stoi(substring);


    // fullmove number
    iss >> substring;
    fullmove_number = std::stoi(substring);

    update_checker_pinned();
}


[[nodiscard]] std::string Board::to_fen() const noexcept {
    std::string fen = "";

    // piece placement
    for (int rank=7; rank>=0; rank--) {
        int empty = 0;
        for (int file=0; file<8; file++) {
            const auto mask = 1ULL << (8*rank+file);

            // empty square?
            if (((white_pieces | black_pieces) & mask) == 0) {
                empty++;
                continue;
            }
            if (empty > 0) {
                fen += std::to_string(empty);
                empty = 0;
            }

            // pieces
            const bool is_white = (white_pieces & mask) != 0;
            if      ((pawns   & mask) != 0) fen += is_white ? "P" : "p";
            else if ((knights & mask) != 0) fen += is_white ? "N" : "n";
            else if ((bishops & mask) != 0) fen += is_white ? "B" : "b";
            else if ((rooks   & mask) != 0) fen += is_white ? "R" : "r";
            else if ((queens  & mask) != 0) fen += is_white ? "Q" : "q";
            else if ((kings   & mask) != 0) fen += is_white ? "K" : "k";
        }
        if (empty > 0) fen += std::to_string(empty);
        if (rank != 0) fen += "/";
    }
    fen += " ";


    // side to move
    if (white_to_move) fen += "w ";
    else fen += "b ";


    // castling rights
    if (castle_rights() == 0) fen += "-";
    else {
        if (castle_rights() & WHITE_SHORT) fen += "K";
        if (castle_rights() & WHITE_LONG) fen += "Q";
        if (castle_rights() & BLACK_SHORT) fen += "k";
        if (castle_rights() & BLACK_LONG) fen += "q";
    }
    fen += " ";


    // en passant
    if (en_passant_target() == -1) fen += "-";
    else fen += int_to_alg(en_passant_target());
    fen += " ";


    // halfmove
    fen += std::to_string(halfmove_clock()) + " ";


    // fullmove
    fen += std::to_string(fullmove_number);
    

    return fen;
}

} // namespace azalea
