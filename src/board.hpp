#pragma once 

#include "move.hpp"

#include <string>
#include <cstdint>

namespace azalea {

typedef std::uint64_t Bitboard;

enum class PieceType {
    Empty,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
};


constexpr char WHITE_SHORT = 8;
constexpr char WHITE_LONG = 4;
constexpr char BLACK_SHORT = 2;
constexpr char BLACK_LONG = 1;


/**
 * Class to represent the board state of a chess game using bitboards for pieces on the
 * board and flags to track certain properties like castling rights.
 */
struct Board {
    Board() = default;
    explicit Board(const std::string_view& fen);
    Board(const Board& other) = default;

    // bitboards
    uint64_t white_pieces{};
    uint64_t black_pieces{};
    uint64_t pawns{};
    uint64_t knights{};
    uint64_t bishops{};
    uint64_t rooks{};
    uint64_t queens{};
    uint64_t kings{};

    // game state properties
    bool white_to_move{true};
    char castle_rights{15}; // white short, white long, black short, black long
    int halfmove_clock{0};
    int fullmove_number{0};
    int en_passant_target{-1};

    void make_move(const Move);
    [[nodiscard]] std::string to_fen() const noexcept;
    [[nodiscard]] PieceType piece_at_square(const int) const noexcept;
    [[nodiscard]] bool is_legal(const Move) const;
    [[nodiscard]] bool is_attacked(const int, const bool) const;
    [[nodiscard]] bool in_check() const;
};

} // namespace azalea
