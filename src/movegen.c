#include "movegen.h"
#include "attacks.h"
#include "handle_move.h"

// Helper functions
inline int get_king_bit(bboard *bb, int color) {
  uint64_t king = (color == WHITE) ? bb->white_king : bb->black_king;
  return __builtin_ctzll(king);
}

// Pawn moves
void generate_pawns_attacks(int *moves, int *index, bboard *bb, int color,
                            uint64_t evasion_mask) {
  uint64_t pawns, enemy_pieces;
  uint64_t empty_squares = ~bb->all_pieces & FULL_MASK;
  uint64_t one_row, start_rank, two_rows;
  uint64_t diag_left, diag_right;
  uint64_t promotion_row;
  int flag;

  int king_bit = get_king_bit(bb, color);

  if (color == WHITE) {
    flag = 1;
    pawns = bb->white_pawns;
    enemy_pieces = bb->black_pieces;
    one_row = (pawns << 8) & empty_squares;
    start_rank = pawns & 0x000000000000FF00ULL;
    two_rows = (((start_rank << 8) & empty_squares) << 8) & empty_squares;
    diag_left = ((pawns & NOT_H_FILE) << 9) & enemy_pieces;
    diag_right = ((pawns & NOT_A_FILE) << 7) & enemy_pieces;
    promotion_row = WHITE_PROMOTION_RANK;
  } else {
    flag = -1;
    pawns = bb->black_pawns;
    enemy_pieces = bb->white_pieces;
    one_row = (pawns >> 8) & empty_squares;
    start_rank = pawns & 0x00FF000000000000ULL;
    two_rows = (((start_rank >> 8) & empty_squares) >> 8) & empty_squares;
    diag_left = ((pawns & NOT_A_FILE) >> 9) & enemy_pieces;
    diag_right = ((pawns & NOT_H_FILE) >> 7) & enemy_pieces;
    promotion_row = BLACK_PROMOTION_RANK;
  }

  // Single push
  while (one_row) {
    uint64_t lowest = one_row & -one_row;
    int to_bit = __builtin_ctzll(lowest);
    int from_bit = to_bit - 8 * flag;
    if (evasion_mask == FULL_MASK ||
        move_is_legal(from_bit, to_bit, evasion_mask)) {
      if (promotion_row & (1ULL << to_bit)) {
        for (int promo = 0; promo < 4; promo++) {
          moves[(*index)++] =
              (from_bit | (to_bit << 6) | (PROMOTION << 12) | (promo << 15));
        }
      } else {
        moves[(*index)++] = (from_bit | (to_bit << 6) | (NORMAL << 12));
      }
    }
    one_row &= one_row - 1;
  }

  // Double push
  while (two_rows) {
    uint64_t lowest = two_rows & -two_rows;
    int to_bit = __builtin_ctzll(lowest);
    int from_bit = to_bit - 16 * flag;
    if (evasion_mask == FULL_MASK ||
        move_is_legal(from_bit, to_bit, evasion_mask)) {
      moves[(*index)++] = (from_bit | (to_bit << 6) | (DOUBLE_MOVE << 12));
    }
    two_rows &= two_rows - 1;
  }

  // Left diagonal capture
  while (diag_left) {
    uint64_t lowest = diag_left & -diag_left;
    int to_bit = __builtin_ctzll(lowest);
    int from_bit = to_bit - 9 * flag;
    if (evasion_mask == FULL_MASK ||
        move_is_legal(from_bit, to_bit, evasion_mask)) {
      if (promotion_row & (1ULL << to_bit)) {
        for (int promo = 0; promo < 4; promo++) {
          moves[(*index)++] =
              (from_bit | (to_bit << 6) | (PROMOTION << 12) | (promo << 15));
        }
      } else {
        moves[(*index)++] = (from_bit | (to_bit << 6) | (NORMAL << 12));
      }
    }
    diag_left &= diag_left - 1;
  }

  // Right diagonal capture
  while (diag_right) {
    uint64_t lowest = diag_right & -diag_right;
    int to_bit = __builtin_ctzll(lowest);
    int from_bit = to_bit - 7 * flag;
    if (evasion_mask == FULL_MASK ||
        move_is_legal(from_bit, to_bit, evasion_mask)) {
      if (promotion_row & (1ULL << to_bit)) {
        for (int promo = 0; promo < 4; promo++) {
          moves[(*index)++] =
              (from_bit | (to_bit << 6) | (PROMOTION << 12) | (promo << 15));
        }
      } else {
        moves[(*index)++] = (from_bit | (to_bit << 6) | (NORMAL << 12));
      }
    }
    diag_right &= diag_right - 1;
  }

  // En passant
  if (bb->en_passant_target != -1) {
    int to_bit = bb->en_passant_target;
    int opponent_pawn = to_bit - 8 * flag;
    int from_left = opponent_pawn - 1;
    if ((from_left % 8) != 7 && (pawns & (1ULL << from_left))) {
      int from_bit = from_left;
      if (king_safe_after_moving(bb, king_bit,
                                 from_bit | (to_bit << 6) | (EN_PASSANT << 12) |
                                     (0 << 15))) {
        moves[(*index)++] = (from_bit | (to_bit << 6) | (EN_PASSANT << 12));
      }
    }
    int from_right = opponent_pawn + 1;
    if ((from_right % 8) != 0 && (pawns & (1ULL << from_right))) {
      int from_bit = from_right;
      if (king_safe_after_moving(bb, king_bit,
                                 from_bit | (to_bit << 6) | (EN_PASSANT << 12) |
                                     (0 << 15))) {
        moves[(*index)++] = (from_bit | (to_bit << 6) | (EN_PASSANT << 12));
      }
    }
  }
}

// Knight moves
void generate_knight_moves(bboard *bb, int color, int *moves, int *count,
                           uint64_t evasion_mask) {
  uint64_t knights = (color == WHITE) ? bb->white_knights : bb->black_knights;
  uint64_t friendly = (color == WHITE) ? bb->white_pieces : bb->black_pieces;
  int king_bit = get_king_bit(bb, color);

  while (knights) {
    uint64_t lb = knights & -knights;
    int from_bit = __builtin_ctzll(lb);
    uint64_t attacks = KNIGHT_ATTACKS[from_bit] & ~friendly;
    while (attacks) {
      uint64_t lb2 = attacks & -attacks;
      int to_bit = __builtin_ctzll(lb2);
      if (evasion_mask == FULL_MASK ||
          move_is_legal(from_bit, to_bit, evasion_mask)) {
        moves[(*count)++] = (from_bit | (to_bit << 6) | (NORMAL << 12));
      }
      attacks &= attacks - 1;
    }
    knights &= knights - 1;
  }
}

// King moves
void generate_king_moves(bboard *bb, int color, int *moves, int *count) {
  int king_first_move =
      (color == WHITE) ? !bb->white_king_moved : !bb->black_king_moved;
  uint64_t pieces = bb->all_pieces;
  uint64_t friendly_pieces;
  int enemy_color;
  uint64_t king;
  uint64_t rook;
  int king_start_sq, castling_info[2][4];

  if (color == WHITE) {
    friendly_pieces = bb->white_pieces;
    enemy_color = BLACK;
    king = bb->white_king;
    rook = bb->white_rooks;
    king_start_sq = 4; // e1
    // array of 2d each d represent a side (queen sie and king side)
    // each side have 4 value (king_after_castl_bit, rook_after_castl_bit,
    // step to decide direction , flag for first move)
    int info[2][4] = {
        {6, 7, 1, bb->white_kingside_rook_moved},  // 6 = g1, 7 = h1
        {2, 0, -1, bb->white_queenside_rook_moved} // 2 = c1, 0 = a1
    };
    memcpy(castling_info, info, sizeof(info));
  } else {
    friendly_pieces = bb->black_pieces;
    enemy_color = WHITE;
    king = bb->black_king;
    rook = bb->black_rooks;
    king_start_sq = 60; // e8
    int info[2][4] = {
        {62, 63, 1, bb->black_kingside_rook_moved},  // 62 = g8, 63 = h8
        {58, 56, -1, bb->black_queenside_rook_moved} // 58 = c8, 56 = a8
    };
    memcpy(castling_info, info, sizeof(info));
  }

  uint64_t lowest1 = king & -king;
  int from_bit = __builtin_ctzll(lowest1);
  uint64_t bitboard = KING_ATTACKS[from_bit] & ~friendly_pieces;

  // Normal king moves
  while (bitboard) {
    uint64_t lowest2 = bitboard & -bitboard;
    int to_bit = __builtin_ctzll(lowest2);
    // Since the king is moving, pass 'from_bit' as the king's current square
    if (king_safe_after_moving(bb, from_bit,
                               from_bit | (to_bit << 6) | (NORMAL << 12) |
                                   (0 << 15))) {
      moves[(*count)++] = (from_bit | (to_bit << 6) | (NORMAL << 12));
    }
    bitboard &= bitboard - 1;
  }

  // Castling : see if the king in check if no test both direction ,
  // if rook moved or captured then stop test on that dorection,
  // if the aquare in the way and the landed one are i cheack stop .
  int castling_possible = 0;
  if (king_first_move && from_bit == king_start_sq &&
      !square_attacked(bb, king_start_sq, enemy_color)) {
    for (int i = 0; i < 2; i++) {
      int castl_bit = castling_info[i][0];
      int rook_bit = castling_info[i][1];
      int step = castling_info[i][2];
      int rook_moved = castling_info[i][3];

      if (!(rook & (1ULL << rook_bit)))
        continue;
      if (rook_moved)
        continue;

      castling_possible = 1;
      int sq = king_start_sq + step;
      int squares_checked = 0;

      while (sq != rook_bit) {
        if (pieces & (1ULL << sq)) {
          castling_possible = 0;
          break;
        }
        squares_checked++;
        if (squares_checked <= 2 && square_attacked(bb, sq, enemy_color)) {
          castling_possible = 0;
          break;
        }
        sq += step;
      }

      if (castling_possible) {
        moves[(*count)++] = (from_bit | (castl_bit << 6) | (CASTLING << 12));
      }
    }
  }
}
// Rook moves (using precompted attack array)
void generate_rook_moves(bboard *bb, int color, int *moves, int *count,
                         uint64_t evasion_mask) {
  uint64_t rooks, friendly_pieces;
  int king_bit = get_king_bit(bb, color);
  if (color == WHITE) {
    rooks = bb->white_rooks;
    friendly_pieces = bb->white_pieces;
  } else {
    rooks = bb->black_rooks;
    friendly_pieces = bb->black_pieces;
  }
  while (rooks) {
    uint64_t lb = rooks & -rooks;
    int from_bit = __builtin_ctzll(lb);
    int index = _pext_u64(bb->all_pieces, ROOK_MASKS[from_bit]);
    uint64_t attacks = ROOK_ATTACKS[from_bit][index] & ~friendly_pieces;
    while (attacks) {
      uint64_t lb2 = attacks & -attacks;
      int to_bit = __builtin_ctzll(lb2);
      if (evasion_mask == FULL_MASK ||
          move_is_legal(from_bit, to_bit, evasion_mask)) {
        moves[(*count)++] = (from_bit | (to_bit << 6) | (NORMAL << 12));
      }
      attacks &= attacks - 1;
    }
    rooks &= rooks - 1;
  }
}

// Bishop moves(using precompted attack array)
void generate_bishop_moves(bboard *bb, int color, int *moves, int *count,
                           uint64_t evasion_mask) {
  uint64_t bishops, friendly_pieces;
  int king_bit = get_king_bit(bb, color);
  if (color == WHITE) {
    bishops = bb->white_bishops;
    friendly_pieces = bb->white_pieces;
  } else {
    bishops = bb->black_bishops;
    friendly_pieces = bb->black_pieces;
  }
  while (bishops) {
    uint64_t lb = bishops & -bishops;
    int from_bit = __builtin_ctzll(lb);
    int index = _pext_u64(bb->all_pieces, BISHOP_MASKS[from_bit]);
    uint64_t attacks = BISHOP_ATTACKS[from_bit][index] & ~friendly_pieces;
    while (attacks) {
      uint64_t lb2 = attacks & -attacks;
      int to_bit = __builtin_ctzll(lb2);
      if (evasion_mask == FULL_MASK ||
          move_is_legal(from_bit, to_bit, evasion_mask)) {
        moves[(*count)++] = (from_bit | (to_bit << 6) | (NORMAL << 12));
      }
      attacks &= attacks - 1;
    }
    bishops &= bishops - 1;
  }
}

// Queen moves (combine rook & bishop precompted attacks)
void generate_queen_moves(bboard *bb, int color, int *moves, int *count,
                          uint64_t evasion_mask) {
  uint64_t queens, friendly_pieces;
  int king_bit = get_king_bit(bb, color);
  if (color == WHITE) {
    queens = bb->white_queen;
    friendly_pieces = bb->white_pieces;
  } else {
    queens = bb->black_queen;
    friendly_pieces = bb->black_pieces;
  }
  while (queens) {
    uint64_t lb = queens & -queens;
    int from_bit = __builtin_ctzll(lb);

    int rook_idx = _pext_u64(bb->all_pieces, ROOK_MASKS[from_bit]);
    uint64_t rook_att = ROOK_ATTACKS[from_bit][rook_idx];

    int bishop_idx = _pext_u64(bb->all_pieces, BISHOP_MASKS[from_bit]);
    uint64_t bishop_att = BISHOP_ATTACKS[from_bit][bishop_idx];

    uint64_t attacks = (rook_att | bishop_att) & ~friendly_pieces;
    while (attacks) {
      uint64_t lb2 = attacks & -attacks;
      int to_bit = __builtin_ctzll(lb2);
      if (evasion_mask == FULL_MASK ||
          move_is_legal(from_bit, to_bit, evasion_mask)) {
        moves[(*count)++] = (from_bit | (to_bit << 6) | (NORMAL << 12));
      }
      attacks &= attacks - 1;
    }
    queens &= queens - 1;
  }
}

// Generate all  moves(all legal moves that don't make king on check)
void generate_all_moves(bboard *bb, int color, int *moves, int *count) {
  *count = 0;
  int king_sq = get_king_bit(bb, color);
  pinned_pieces(bb, king_sq, color);
  uint64_t checkers = get_checkers(bb, king_sq, color);
  int num_checkers = __builtin_popcountll(checkers);

  if (num_checkers >= 2) {
    generate_king_moves(bb, color, moves, count);
    return;
  }

  uint64_t evasion_mask = 0;
  if (num_checkers == 1) {

    evasion_mask = checkers;
    int checker_bit = __builtin_ctzll(checkers);
    int unused_var, piece;
    uint64_t *unused_bb = get_piece_bb(bb, checker_bit, &unused_var, &piece);
    if (piece == 2 || piece == 3 || piece == 4) {
      evasion_mask |= PATH_BETWEEN[king_sq][checker_bit];
    }
  } else if (PINNED_PIECES == 0)
    evasion_mask = FULL_MASK;

  generate_pawns_attacks(moves, count, bb, color, evasion_mask);
  generate_knight_moves(bb, color, moves, count, evasion_mask);
  generate_king_moves(bb, color, moves, count);
  generate_rook_moves(bb, color, moves, count, evasion_mask);
  generate_bishop_moves(bb, color, moves, count, evasion_mask);
  generate_queen_moves(bb, color, moves, count, evasion_mask);
}
