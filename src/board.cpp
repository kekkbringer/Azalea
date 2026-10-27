#include "board.hpp"
#include "attacks.hpp"
#include "util.hpp"
#include "zobrist.hpp"

#include <iostream>
#include <sstream>

namespace azalea {

[[nodiscard]] bool Board::is_repetition() const {
    const int n_hist = static_cast<int>(position_history.size());
    // only the last 'halfmove_clock' positions can even be a repetition
    const int stop = std::max(0, n_hist - 1 - halfmove_clock());
    // go in increments of 2, to use asymmetry of side_to_move
    for (int i=n_hist-3; i >= stop; i-=2)
        if (position_history[static_cast<std::size_t>(i)].hash == hash()) return true;
    return false;
}

[[nodiscard]] bool Board::in_check() const {
    const Bitboard& us = white_to_move ? white_pieces : black_pieces;
    const int king_sq = lsb(us & kings);
    return is_attacked(king_sq, not white_to_move);
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

[[nodiscard]] bool Board::is_legal(const Move move) {
    if (move.is_king_side_castle() or move.is_queen_side_castle()) {
        const int middle = (move.from() + move.to()) / 2;   // f1/d1 or f8/d8
        if (is_attacked(move.from(), not white_to_move) or
            is_attacked(middle, not white_to_move)) return false;
    }
    make_move(move);
    const Bitboard& mover = white_to_move ? black_pieces : white_pieces;
    const bool legal = not is_attacked(lsb(mover & kings), white_to_move);
    unmake_move(move);
    return legal;
}

void Board::unmake_move(const Move move) {
    const PieceType captured_piece = captured();
    position_history.pop_back();

    const Bitboard from_mask = (1ULL << move.from());
    const Bitboard to_mask = (1ULL << move.to());

    // move piece that was move back
    Bitboard& them = white_to_move ? white_pieces : black_pieces;
    Bitboard& us   = white_to_move ? black_pieces : white_pieces;
    us   ^= from_mask | to_mask;
    if (move.is_capture()) them |= to_mask;

    const auto moved_piece = piece_at_square(move.to());

    switch (moved_piece) {
        case PieceType::Pawn:   { pawns   |= from_mask; pawns   ^= to_mask; break; }
        case PieceType::Knight: { knights |= from_mask; knights ^= to_mask; break; }
        case PieceType::Bishop: { bishops |= from_mask; bishops ^= to_mask; break; }
        case PieceType::Rook:   { rooks   |= from_mask; rooks   ^= to_mask; break; }
        case PieceType::Queen:  { queens  |= from_mask; queens  ^= to_mask; break; }
        case PieceType::King:   { kings   |= from_mask; kings   ^= to_mask; break; }
        case PieceType::Empty:  break;
    }

    switch (captured_piece) {
        case PieceType::Pawn:   { pawns   |= to_mask; break; }
        case PieceType::Knight: { knights |= to_mask; break; }
        case PieceType::Bishop: { bishops |= to_mask; break; }
        case PieceType::Rook:   { rooks   |= to_mask; break; }
        case PieceType::Queen:  { queens  |= to_mask; break; }
        case PieceType::King:   { kings   |= to_mask; break; }
        case PieceType::Empty:  break;
    }

    if (move.is_promotion()) {
        if (move.is_knight_promotion()) { knights ^= from_mask; pawns ^= from_mask; }
        if (move.is_bishop_promotion()) { bishops ^= from_mask; pawns ^= from_mask; }
        if (move.is_rook_promotion())   { rooks   ^= from_mask; pawns ^= from_mask; }
        if (move.is_queen_promotion())  { queens  ^= from_mask; pawns ^= from_mask; }
    }

    if (move.is_en_passant()) {
        const Bitboard shifted = white_to_move ? (to_mask << 8) : (to_mask >> 8);
        pawns ^= to_mask; // fix from normal capture
        them  ^= to_mask; // fix from normal capture
        pawns ^= shifted;
        them  ^= shifted;
    }

    if (move.is_king_side_castle()) {
        if (not white_to_move) {
            rooks ^= (1ULL << F1) | (1ULL << H1);
            white_pieces ^= (1ULL << F1) | (1ULL << H1);
        } else { // black to move
            rooks ^= (1ULL << F8) | (1ULL << H8);
            black_pieces ^= (1ULL << F8) | (1ULL << H8);
        }
    } else if (move.is_queen_side_castle()) {
        if (not white_to_move) {
            rooks ^= (1ULL << A1) | (1ULL << D1);
            white_pieces ^= (1ULL << A1) | (1ULL << D1);
        } else { // black to move
            rooks ^= (1ULL << A8) | (1ULL << D8);
            black_pieces ^= (1ULL << A8) | (1ULL << D8);
        }
    }

    if (white_to_move) --fullmove_number;

    white_to_move = not white_to_move;
}

void Board::make_move(const Move move) {
    position_history.push_back(position_history.back());

    const Bitboard from_mask = (1ULL << move.from());
    const Bitboard to_mask = (1ULL << move.to());

    // move piece
    const auto moved_piece = piece_at_square(move.from());
    const auto captured_piece = piece_at_square(move.to());
    captured() = captured_piece;

    Bitboard& us   = white_to_move ? white_pieces : black_pieces;
    Bitboard& them = white_to_move ? black_pieces : white_pieces;
    us   ^= from_mask | to_mask;
    them &= ~to_mask;

    const size_t our_color_offset   = white_to_move ? 0 : 6;
    const size_t their_color_offset = white_to_move ? 6 : 0;

    switch (captured_piece) {
        case PieceType::Pawn:   { pawns   &= ~to_mask; hash() ^= z_keys.piece_square[their_color_offset+0][move.to()]; break; }
        case PieceType::Knight: { knights &= ~to_mask; hash() ^= z_keys.piece_square[their_color_offset+1][move.to()]; break; }
        case PieceType::Bishop: { bishops &= ~to_mask; hash() ^= z_keys.piece_square[their_color_offset+2][move.to()]; break; }
        case PieceType::Rook:   { rooks   &= ~to_mask; hash() ^= z_keys.piece_square[their_color_offset+3][move.to()]; break; }
        case PieceType::Queen:  { queens  &= ~to_mask; hash() ^= z_keys.piece_square[their_color_offset+4][move.to()]; break; }
        case PieceType::King:   { kings   &= ~to_mask; hash() ^= z_keys.piece_square[their_color_offset+5][move.to()]; break; }
        case PieceType::Empty:  break;
    }

    switch (moved_piece) {
        case PieceType::Pawn:   { pawns   ^= from_mask; pawns   |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+0][move.to()]; hash() ^= z_keys.piece_square[our_color_offset+0][move.from()]; break; }
        case PieceType::Knight: { knights ^= from_mask; knights |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+1][move.to()]; hash() ^= z_keys.piece_square[our_color_offset+1][move.from()]; break; }
        case PieceType::Bishop: { bishops ^= from_mask; bishops |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+2][move.to()]; hash() ^= z_keys.piece_square[our_color_offset+2][move.from()]; break; }
        case PieceType::Rook:   { rooks   ^= from_mask; rooks   |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+3][move.to()]; hash() ^= z_keys.piece_square[our_color_offset+3][move.from()]; break; }
        case PieceType::Queen:  { queens  ^= from_mask; queens  |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+4][move.to()]; hash() ^= z_keys.piece_square[our_color_offset+4][move.from()]; break; }
        case PieceType::King:   { kings   ^= from_mask; kings   |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+5][move.to()]; hash() ^= z_keys.piece_square[our_color_offset+5][move.from()]; break; }
        case PieceType::Empty:  break;
    }

    if (move.is_en_passant()) {
        const Bitboard shifted = white_to_move ? (to_mask >> 8) : (to_mask << 8);
        pawns &= ~shifted;
        them  &= ~shifted;
        captured() = PieceType::Pawn;
        // fix wrong hash modification and do the proper one
        hash() ^= z_keys.piece_square[their_color_offset+0][static_cast<std::size_t>(lsb(shifted))];
    }

    if (move.is_king_side_castle()) {
        if (white_to_move) {
            rooks ^= (1ULL << F1) | (1ULL << H1);
            white_pieces ^= (1ULL << F1) | (1ULL << H1);
            hash() ^= z_keys.piece_square[3][F1];
            hash() ^= z_keys.piece_square[3][H1];
        } else { // black to move
            rooks ^= (1ULL << F8) | (1ULL << H8);
            black_pieces ^= (1ULL << F8) | (1ULL << H8);
            hash() ^= z_keys.piece_square[9][F8];
            hash() ^= z_keys.piece_square[9][H8];
        }
    } else if (move.is_queen_side_castle()) {
        if (white_to_move) {
            rooks ^= (1ULL << A1) | (1ULL << D1);
            white_pieces ^= (1ULL << A1) | (1ULL << D1);
            hash() ^= z_keys.piece_square[3][A1];
            hash() ^= z_keys.piece_square[3][D1];
        } else { // black to move
            rooks ^= (1ULL << A8) | (1ULL << D8);
            black_pieces ^= (1ULL << A8) | (1ULL << D8);
            hash() ^= z_keys.piece_square[9][A8];
            hash() ^= z_keys.piece_square[9][D8];
        }
    }
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

    if (move.is_promotion()) {
        pawns ^= to_mask;
        hash() ^= z_keys.piece_square[our_color_offset][move.to()];
        if (move.is_knight_promotion()) { knights |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+1][move.to()]; }
        if (move.is_bishop_promotion()) { bishops |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+2][move.to()]; }
        if (move.is_rook_promotion())   { rooks   |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+3][move.to()]; }
        if (move.is_queen_promotion())  { queens  |= to_mask; hash() ^= z_keys.piece_square[our_color_offset+4][move.to()]; }
    }

    if (en_passant_target() != -1)
        hash() ^= z_keys.en_passant[static_cast<std::size_t>(en_passant_target()%8)];
    en_passant_target() = -1;

    if (move.is_double_pawn_push()) {
        const int ep_sq = (move.to() + move.from()) / 2;
        // only set en_passant target if a capture is possible, otherwise it needs
        // to be identical to the position where the double pawn push did not
        // occur on the last move
        const Bitboard capturers = attacks::pawn(white_to_move, ep_sq) & pawns & them;
        if (capturers != 0) {
            en_passant_target() = ep_sq;
            hash() ^= z_keys.en_passant[static_cast<std::size_t>(move.to()%8)];
        }
    }

    ++halfmove_clock();
    if (move.is_capture() or moved_piece == PieceType::Pawn) halfmove_clock() = 0;

    if (not white_to_move) ++fullmove_number;

    white_to_move = not white_to_move;
    hash() ^= z_keys.side_to_move;
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

            case 'P': { hash() ^= z_keys.piece_square[ 0][static_cast<std::size_t>(index)]; ++line_cap; white_pieces |= (1ULL<<index); pawns |= (1ULL<<index); ++index; continue;}
            case 'N': { hash() ^= z_keys.piece_square[ 1][static_cast<std::size_t>(index)]; ++line_cap; white_pieces |= (1ULL<<index); knights |= (1ULL<<index); ++index; continue; }
            case 'B': { hash() ^= z_keys.piece_square[ 2][static_cast<std::size_t>(index)]; ++line_cap; white_pieces |= (1ULL<<index); bishops |= (1ULL<<index); ++index; continue; }
            case 'R': { hash() ^= z_keys.piece_square[ 3][static_cast<std::size_t>(index)]; ++line_cap; white_pieces |= (1ULL<<index); rooks |= (1ULL<<index); ++index; continue; }
            case 'Q': { hash() ^= z_keys.piece_square[ 4][static_cast<std::size_t>(index)]; ++line_cap; white_pieces |= (1ULL<<index); queens |= (1ULL<<index); ++index; continue; }
            case 'K': { hash() ^= z_keys.piece_square[ 5][static_cast<std::size_t>(index)]; ++line_cap; white_pieces |= (1ULL<<index); kings |= (1ULL<<index); ++index; continue; }
            case 'p': { hash() ^= z_keys.piece_square[ 6][static_cast<std::size_t>(index)]; ++line_cap; black_pieces |= (1ULL<<index); pawns |= (1ULL<<index); ++index; continue; }
            case 'n': { hash() ^= z_keys.piece_square[ 7][static_cast<std::size_t>(index)]; ++line_cap; black_pieces |= (1ULL<<index); knights |= (1ULL<<index); ++index; continue; }
            case 'b': { hash() ^= z_keys.piece_square[ 8][static_cast<std::size_t>(index)]; ++line_cap; black_pieces |= (1ULL<<index); bishops |= (1ULL<<index); ++index; continue; }
            case 'r': { hash() ^= z_keys.piece_square[ 9][static_cast<std::size_t>(index)]; ++line_cap; black_pieces |= (1ULL<<index); rooks |= (1ULL<<index); ++index; continue; }
            case 'q': { hash() ^= z_keys.piece_square[10][static_cast<std::size_t>(index)]; ++line_cap; black_pieces |= (1ULL<<index); queens |= (1ULL<<index); ++index; continue; }
            case 'k': { hash() ^= z_keys.piece_square[11][static_cast<std::size_t>(index)]; ++line_cap; black_pieces |= (1ULL<<index); kings |= (1ULL<<index); ++index; continue; }
            default: {
                         std::cout << "Unknown symbol in FEN" << std::endl;
                         return;
                     }
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
