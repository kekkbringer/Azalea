#include "board.hpp"
#include "attacks.hpp"
#include "util.hpp"

#include <iostream>
#include <sstream>

namespace azalea {

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

[[nodiscard]] bool Board::is_legal(const Move move) const {
    if (move.is_king_side_castle() or move.is_queen_side_castle()) {
        const int middle = (move.from() + move.to()) / 2;   // f1/d1 or f8/d8
        if (is_attacked(move.from(), not white_to_move) or
            is_attacked(middle, not white_to_move)) return false;
    }
    Board copy = *this;
    copy.make_move(move);
    const Bitboard& us = white_to_move ? copy.white_pieces : copy.black_pieces;
    const int king_sq = lsb(us & copy.kings);
    return not copy.is_attacked(king_sq, not white_to_move);
}

void Board::make_move(const Move move) {
    const Bitboard from_mask = (1ULL << move.from());
    const Bitboard to_mask = (1ULL << move.to());

    // move piece
    const auto moved_piece = piece_at_square(move.from());
    const auto captured_piece = piece_at_square(move.to());

    Bitboard& us   = white_to_move ? white_pieces : black_pieces;
    Bitboard& them = white_to_move ? black_pieces : white_pieces;
    us   ^= from_mask | to_mask;
    them &= ~to_mask;

    switch (captured_piece) {
        case PieceType::Pawn:   { pawns   &= ~to_mask; break; }
        case PieceType::Knight: { knights &= ~to_mask; break; }
        case PieceType::Bishop: { bishops &= ~to_mask; break; }
        case PieceType::Rook:   { rooks   &= ~to_mask; break; }
        case PieceType::Queen:  { queens  &= ~to_mask; break; }
        case PieceType::King:   { kings   &= ~to_mask; break; }
        case PieceType::Empty:  break;
    }

    switch (moved_piece) {
        case PieceType::Pawn:   { pawns   ^= from_mask; pawns   |= to_mask; break; }
        case PieceType::Knight: { knights ^= from_mask; knights |= to_mask; break; }
        case PieceType::Bishop: { bishops ^= from_mask; bishops |= to_mask; break; }
        case PieceType::Rook:   { rooks   ^= from_mask; rooks   |= to_mask; break; }
        case PieceType::Queen:  { queens  ^= from_mask; queens  |= to_mask; break; }
        case PieceType::King:   { kings   ^= from_mask; kings   |= to_mask; break; }
        case PieceType::Empty:  break;
    }

    if (move.is_en_passant()) {
        const Bitboard shifted = white_to_move ? (to_mask >> 8) : (to_mask << 8);
        pawns &= ~shifted;
        them  &= ~shifted;
    }

    if (move.is_king_side_castle()) {
        if (white_to_move) {
            rooks ^= (1ULL << F1) | (1ULL << H1);
            white_pieces ^= (1ULL << F1) | (1ULL << H1);
        } else { // black to move
            rooks ^= (1ULL << F8) | (1ULL << H8);
            black_pieces ^= (1ULL << F8) | (1ULL << H8);
        }
    } else if (move.is_queen_side_castle()) {
        if (white_to_move) {
            rooks ^= (1ULL << A1) | (1ULL << D1);
            white_pieces ^= (1ULL << A1) | (1ULL << D1);
        } else { // black to move
            rooks ^= (1ULL << A8) | (1ULL << D8);
            black_pieces ^= (1ULL << A8) | (1ULL << D8);
        }
    }
    if (move.from() == E1)
        castle_rights &= ~(WHITE_SHORT | WHITE_LONG);
    else if (move.from() == A1 or move.to() == A1)
        castle_rights &= ~WHITE_LONG;
    else if (move.from() == H1 or move.to() == H1)
        castle_rights &= ~WHITE_SHORT;
    if (move.from() == E8)
        castle_rights &= ~(BLACK_SHORT | BLACK_LONG);
    else if (move.from() == A8 or move.to() == A8)
        castle_rights &= ~BLACK_LONG;
    else if (move.from() == H8 or move.to() == H8)
        castle_rights &= ~BLACK_SHORT;

    if (move.is_promotion()) {
        pawns ^= to_mask;
        if (move.is_knight_promotion()) knights |= to_mask;
        if (move.is_bishop_promotion()) bishops |= to_mask;
        if (move.is_rook_promotion())   rooks   |= to_mask;
        if (move.is_queen_promotion())  queens  |= to_mask;
    }

    if (move.is_double_pawn_push()) en_passant_target = (move.to() + move.from()) / 2;
    else en_passant_target = -1;

    ++halfmove_clock;
    if (move.is_capture() or moved_piece == PieceType::Pawn) halfmove_clock = 0;

    if (not white_to_move) ++fullmove_number;

    white_to_move = not white_to_move;
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


/**
 * Initializes the board from a FEN string.
 * TODO: Properly reject invalid FEN strings.
 */
Board::Board(const std::string_view& fen) {
    std::istringstream iss(std::string{fen});
    std::string substring;

    // piece placement
    iss >> substring;
    int index = 56;
    int line_cap = 0;
    for (const auto& c: substring) {
        switch (c) {
            case '1': { line_cap += 1; index += 1; continue; }
            case '2': { line_cap += 2; index += 2; continue; }
            case '3': { line_cap += 3; index += 3; continue; }
            case '4': { line_cap += 4; index += 4; continue; }
            case '5': { line_cap += 5; index += 5; continue; }
            case '6': { line_cap += 6; index += 6; continue; }
            case '7': { line_cap += 7; index += 7; continue; }
            case '8': { line_cap += 8; index += 8; continue; }
            case '/': {
                          if (line_cap == 8) {
                              line_cap = 0;
                              index -= 16;
                              continue;
                          }
                          else {
                              std::cout << "Invalid FEN" << std::endl;
                              return;
                          }
                      }

            case 'P': { ++line_cap; white_pieces |= (1ULL<<index); pawns |= (1ULL<<index); ++index; continue;}
            case 'N': { ++line_cap; white_pieces |= (1ULL<<index); knights |= (1ULL<<index); ++index; continue; }
            case 'B': { ++line_cap; white_pieces |= (1ULL<<index); bishops |= (1ULL<<index); ++index; continue; }
            case 'R': { ++line_cap; white_pieces |= (1ULL<<index); rooks |= (1ULL<<index); ++index; continue; }
            case 'Q': { ++line_cap; white_pieces |= (1ULL<<index); queens |= (1ULL<<index); ++index; continue; }
            case 'K': { ++line_cap; white_pieces |= (1ULL<<index); kings |= (1ULL<<index); ++index; continue; }
            case 'p': { ++line_cap; black_pieces |= (1ULL<<index); pawns |= (1ULL<<index); ++index; continue; }
            case 'n': { ++line_cap; black_pieces |= (1ULL<<index); knights |= (1ULL<<index); ++index; continue; }
            case 'b': { ++line_cap; black_pieces |= (1ULL<<index); bishops |= (1ULL<<index); ++index; continue; }
            case 'r': { ++line_cap; black_pieces |= (1ULL<<index); rooks |= (1ULL<<index); ++index; continue; }
            case 'q': { ++line_cap; black_pieces |= (1ULL<<index); queens |= (1ULL<<index); ++index; continue; }
            case 'k': { ++line_cap; black_pieces |= (1ULL<<index); kings |= (1ULL<<index); ++index; continue; }
            default: {
                         std::cout << "Unknown symbol in FEN" << std::endl;
                         return;
                     }
        }
    }


    // side to move
    iss >> substring;
    if (substring == "w") white_to_move = true;
    else if (substring == "b") white_to_move = false;
    else {
        std::cout << "Invalid FEN" << std::endl;
        return;
    }


    // castling rights
    iss >> substring;
    castle_rights = 0;
    for (const auto& c: substring) {
        if (substring == "-") break;
        switch (c) {
            case 'K': { castle_rights |= WHITE_SHORT; continue; }
            case 'Q': { castle_rights |= WHITE_LONG; continue; }
            case 'k': { castle_rights |= BLACK_SHORT; continue; }
            case 'q': { castle_rights |= BLACK_LONG; continue; }
            default: {
                         std::cout << "Unknown symbol in FEN" << std::endl;
                         return;
                     }
        }
    }


    // en passant target
    iss >> substring;
    if (substring == "-") en_passant_target = -1;
    else en_passant_target = alg_to_int(substring);


    // halfmove clock
    iss >> substring;
    halfmove_clock = std::stoi(substring);


    // fullmove number
    iss >> substring;
    fullmove_number = std::stoi(substring);
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
    if (castle_rights == 0) fen += "-";
    else {
        if (castle_rights & WHITE_SHORT) fen += "K";
        if (castle_rights & WHITE_LONG) fen += "Q";
        if (castle_rights & BLACK_SHORT) fen += "k";
        if (castle_rights & BLACK_LONG) fen += "q";
    }
    fen += " ";


    // en passant
    if (en_passant_target == -1) fen += "-";
    else fen += int_to_alg(en_passant_target);
    fen += " ";


    // halfmove
    fen += std::to_string(halfmove_clock) + " ";


    // fullmove
    fen += std::to_string(fullmove_number);
    

    return fen;
}

} // namespace azalea
