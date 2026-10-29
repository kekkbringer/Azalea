#pragma once 

#include "move.hpp"

#include <string>
#include <cstdint>
#include <vector>

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


constexpr std::uint8_t WHITE_SHORT = 8;
constexpr std::uint8_t WHITE_LONG = 4;
constexpr std::uint8_t BLACK_SHORT = 2;
constexpr std::uint8_t BLACK_LONG = 1;


/**
 * Keeps all information about the position that need to be restored when unmaking
 * a move.
 */
struct PositionInfo {
    std::uint64_t hash{0};
    int en_passant_target{-1};
    std::uint8_t castle_rights{15};
    int halfmove_clock{0};
    PieceType captured{PieceType::Empty};
    Bitboard checkers{0ULL};
    Bitboard pinned{0ULL};
    int pliesFromLastNull{124}; // something "large" to allow null move pruning early
};


/**
 * Class to represent the board state of a chess game using bitboards for pieces on the
 * board and flags to track certain properties like castling rights.
 */
struct Board {
    Board();
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
    std::vector<PositionInfo> position_history;
    bool white_to_move{true};
    int fullmove_number{0};

    [[nodiscard]] std::uint64_t& hash() { return position_history.back().hash; }
    [[nodiscard]] int& en_passant_target() { return position_history.back().en_passant_target; }
    [[nodiscard]] std::uint8_t& castle_rights() { return position_history.back().castle_rights; }
    [[nodiscard]] int& halfmove_clock() { return position_history.back().halfmove_clock; }
    [[nodiscard]] PieceType& captured() { return position_history.back().captured; }
    [[nodiscard]] std::uint64_t hash() const { return position_history.back().hash; }
    [[nodiscard]] int en_passant_target() const { return position_history.back().en_passant_target; }
    [[nodiscard]] std::uint8_t castle_rights() const { return position_history.back().castle_rights; }
    [[nodiscard]] int halfmove_clock() const { return position_history.back().halfmove_clock; }
    [[nodiscard]] PieceType captured() const { return position_history.back().captured; }

    void make_move(const Move);
    void unmake_move(const Move);
    void make_null_move();
    void unmake_null_move();
    void update_checker_pinned();
    [[nodiscard]] Bitboard attacking_to (const int, const Bitboard) const noexcept;
    [[nodiscard]] std::string to_fen() const noexcept;
    [[nodiscard]] PieceType piece_at_square(const int) const noexcept;
    [[nodiscard]] bool is_legal (const Move) const;
    [[nodiscard]] bool is_pseudolegal (const Move) const;
    [[nodiscard]] bool is_attacked(const int, const bool) const;
    [[nodiscard]] bool in_check() const { return position_history.back().checkers != 0; }
    [[nodiscard]] bool is_repetition() const;
    [[nodiscard]] bool player_has_non_pawn_material() const;
    [[nodiscard]] bool gives_check(const Move);
};

} // namespace azalea
