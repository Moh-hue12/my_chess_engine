#include "hundle_move.h"
#include "evaluation.h"
#include "movegen.h"

const int ROOK_CASTLE_FROM[2][2] = {
    {0, 7},  // white: queenside (a1), kingside (h1)
    {56, 63} // black: queenside (a8), kingside (h8)
};

const int ROOK_CASTLE_TO[2][2] = {
    {3, 5},  // white: queenside → d1, kingside → f1
    {59, 61} // black: queenside → d8, kingside → f8
};

static const int PhaseWeight[5] = {0, 1, 1, 2, 4};

// return the piece information ( its board addr to make change dairectly on it
// , and the a piece flag to test for king moves and rooks)
uint64_t *get_piece_bb(bboard *bb, int sq, int *color, int *piece) {
  uint64_t mask = 1ULL << sq;
  if (bb->white_pieces & mask) {
    if (color)
      *color = WHITE;
    if (bb->white_pawns & mask) {
      if (piece)
        *piece = 0;
      return &bb->white_pawns;
    }
    if (bb->white_knights & mask) {
      if (piece)
        *piece = 1;
      return &bb->white_knights;
    }
    if (bb->white_bishops & mask) {
      if (piece)
        *piece = 2;
      return &bb->white_bishops;
    }
    if (bb->white_rooks & mask) {
      if (piece)
        *piece = 3;
      return &bb->white_rooks;
    }
    if (bb->white_queen & mask) {
      if (piece)
        *piece = 4;
      return &bb->white_queen;
    }
    if (bb->white_king & mask) {
      if (piece)
        *piece = 5;
      return &bb->white_king;
    }
  } else {
    if (color)
      *color = BLACK;
    if (bb->black_pawns & mask) {
      if (piece)
        *piece = 0;
      return &bb->black_pawns;
    }
    if (bb->black_knights & mask) {
      if (piece)
        *piece = 1;
      return &bb->black_knights;
    }
    if (bb->black_bishops & mask) {
      if (piece)
        *piece = 2;
      return &bb->black_bishops;
    }
    if (bb->black_rooks & mask) {
      if (piece)
        *piece = 3;
      return &bb->black_rooks;
    }
    if (bb->black_queen & mask) {
      if (piece)
        *piece = 4;
      return &bb->black_queen;
    }
    if (bb->black_king & mask) {
      if (piece)
        *piece = 5;
      return &bb->black_king;
    }
  }
  return NULL;
}

static void clear_square(bboard *bb, int sq) {
  int color, piece;
  uint64_t *bb_piece = get_piece_bb(bb, sq, &color, &piece);
  if (!bb_piece)
    return;
  uint64_t mask = ~(1ULL << sq);
  *bb_piece &= mask;
  if (color == WHITE) {
    bb->white_pieces &= mask;
  } else {
    bb->black_pieces &= mask;
  }
  bb->all_pieces = bb->white_pieces | bb->black_pieces;
}

// this funciton hundle two things it clear the piece old postion and move the
// piece to its destintion (i dind't call clear_sqaure here to avoid  function
// call, i can clear and set bit in same time with simple bit operation)
static void move_piece(bboard *bb, int from_bit, int to_bit, int color,
                       int piece) {
  uint64_t from_mask = ~(1ULL << from_bit);
  uint64_t to_mask = 1ULL << to_bit;
  uint64_t *bb_piece = get_piece_bb(bb, from_bit, NULL, NULL);
  if (!bb_piece)
    return;
  *bb_piece = (*bb_piece & from_mask) | to_mask;
  if (color == WHITE) {
    bb->white_pieces = (bb->white_pieces & from_mask) | to_mask;
  } else {
    bb->black_pieces = (bb->black_pieces & from_mask) | to_mask;
  }
  bb->all_pieces = bb->white_pieces | bb->black_pieces;
}

// // make_move : copy the board an do the move temporary
// int make_move(bboard *bb, int from_bit, int to_bit, int action, int
// promo_piece,
//               bboard *state_copy) {
//   *state_copy = *bb;
//   bb->turn *= -1;
//
//   int color, piece;
//   uint64_t *moving_bb = get_piece_bb(bb, from_bit, &color, &piece);
//   if (!moving_bb)
//     return color;
//
//   uint64_t from_mask = ~(1ULL << from_bit);
//   uint64_t to_mask = 1ULL << to_bit;
//
//   int flag;
//   int enemy_color;
//   uint64_t enemy_pieces;
//   if (color == WHITE) {
//     flag = 1;
//     enemy_color = BLACK;
//     enemy_pieces = bb->black_pieces;
//     if (piece == 5)
//       bb->white_king_moved = true;
//   } else {
//     flag = -1;
//     enemy_color = WHITE;
//     enemy_pieces = bb->white_pieces;
//     if (piece == 5)
//       bb->black_king_moved = true;
//   }
//
//   // if capture move remove the captrued piece
//   if (action != EN_PASSANT) {
//     if (enemy_pieces & to_mask) {
//       clear_square(bb, to_bit);
//     }
//   }
//
//   // Update castling rights if a rook moves or is captured
//   // ( if this trigger for other piece that mean the varaible are aready
//   True) if (from_bit == 0 || to_bit == 0) {
//     bb->white_queenside_rook_moved = true;
//   }
//   if (from_bit == 7 || to_bit == 7) {
//     bb->white_kingside_rook_moved = true;
//   }
//   if (from_bit == 56 || to_bit == 56) {
//     bb->black_queenside_rook_moved = true;
//   }
//   if (from_bit == 63 || to_bit == 63) {
//     bb->black_kingside_rook_moved = true;
//   }
//
//   move_piece(bb, from_bit, to_bit, color, piece);
//   bb->en_passant_target = -1;
//
//   if (action == CASTLING) {
//     // decide if we are moving right or left
//     int info = (from_bit - to_bit > 0) ? 0 : 1; // 0=queenside, 1=kingside
//     int rook_from = ROOK_CASTLE_FROM[color][info];
//     int rook_to = ROOK_CASTLE_TO[color][info];
//     move_piece(bb, rook_from, rook_to, color, 3);
//     if (color == WHITE) {
//       if (info == 0)
//         bb->white_queenside_rook_moved = true;
//       else
//         bb->white_kingside_rook_moved = true;
//     } else {
//       if (info == 0)
//         bb->black_queenside_rook_moved = true;
//       else
//         bb->black_kingside_rook_moved = true;
//     }
//   } else if (action == PROMOTION) {
//     clear_square(bb, to_bit);
//     if (color == WHITE) {
//       bb->white_pieces |= to_mask;
//       if (promo_piece == PROMOTION_KNIGHT)
//         bb->white_knights |= to_mask;
//       else if (promo_piece == PROMOTION_BISHOP)
//         bb->white_bishops |= to_mask;
//       else if (promo_piece == PROMOTION_ROOK)
//         bb->white_rooks |= to_mask;
//       else if (promo_piece == PROMOTION_QUEEN)
//         bb->white_queen |= to_mask;
//     } else {
//       bb->black_pieces |= to_mask;
//       if (promo_piece == PROMOTION_KNIGHT)
//         bb->black_knights |= to_mask;
//       else if (promo_piece == PROMOTION_BISHOP)
//         bb->black_bishops |= to_mask;
//       else if (promo_piece == PROMOTION_ROOK)
//         bb->black_rooks |= to_mask;
//       else if (promo_piece == PROMOTION_QUEEN)
//         bb->black_queen |= to_mask;
//     }
//     bb->all_pieces = bb->white_pieces | bb->black_pieces;
//   } else if (action == EN_PASSANT) {
//     clear_square(bb, to_bit - 8 * flag);
//   } else if (action == DOUBLE_MOVE) {
//     bb->en_passant_target = to_bit - 8 * flag;
//   }
//
//   return color;
// }
// past the cpeid board on make move to the original one
// void undo_move(bboard *bb, bboard *state_copy) { *bb = *state_copy; }

int make_move(bboard *bb, int move, UndoInfo *u) {

  int from_bit = move & FROM_MASK;
  int to_bit = (move & TO_MASK) >> 6;
  int action = (move & ACTION_MASK) >> 12;
  int promo_piece = (move & PROMO_MASK) >> 15;

  // Save the complete undo state before any changes
  u->from = from_bit;
  u->to = to_bit;
  u->action = action;
  u->promo = promo_piece;
  u->en_passant_target = bb->en_passant_target;
  u->white_king_moved = bb->white_king_moved;
  u->black_king_moved = bb->black_king_moved;
  u->white_kingside_rook_moved = bb->white_kingside_rook_moved;
  u->white_queenside_rook_moved = bb->white_queenside_rook_moved;
  u->black_kingside_rook_moved = bb->black_kingside_rook_moved;
  u->black_queenside_rook_moved = bb->black_queenside_rook_moved;
  u->old_mg_score = mg_score;
  u->old_eg_score = eg_score;

  // Switch turn
  bb->turn *= -1;

  // Determine moving piece and color
  int color, piece;
  uint64_t *moving_bb = get_piece_bb(bb, from_bit, &color, &piece);
  if (!moving_bb)
    return color;

  u->color = color;
  u->moving_piece = piece;

  uint64_t from_mask = ~(1ULL << from_bit);
  uint64_t to_mask = 1ULL << to_bit;

  int flag;
  int enemy_color;
  uint64_t enemy_pieces;
  if (color == WHITE) {
    flag = 1;
    enemy_color = BLACK;
    enemy_pieces = bb->black_pieces;
    if (piece == 5)
      bb->white_king_moved = true;
  } else {
    flag = -1;
    enemy_color = WHITE;
    enemy_pieces = bb->white_pieces;
    if (piece == 5)
      bb->black_king_moved = true;
  }

  // Initialize captured info
  u->captured_piece = -1;
  u->captured_sq = -1;

  // Normal capture detection (en passant is handled later)
  if (action != EN_PASSANT && (enemy_pieces & to_mask)) {
    get_piece_bb(bb, to_bit, NULL, &u->captured_piece);
    u->captured_sq = to_bit;
    clear_square(bb, to_bit);
    if (u->captured_piece <= 4) {
      bb->game_phase -= PhaseWeight[u->captured_piece];
    }
  }

  // Update castling rights if a rook moves or is captured
  if (from_bit == 0 || to_bit == 0)
    bb->white_queenside_rook_moved = true;
  if (from_bit == 7 || to_bit == 7)
    bb->white_kingside_rook_moved = true;
  if (from_bit == 56 || to_bit == 56)
    bb->black_queenside_rook_moved = true;
  if (from_bit == 63 || to_bit == 63)
    bb->black_kingside_rook_moved = true;

  // Move the piece
  move_piece(bb, from_bit, to_bit, color, piece);
  bb->en_passant_target = -1;

  // Handle special actions
  if (action == CASTLING) {
    int info = (from_bit > to_bit) ? 0 : 1;
    int rook_from = ROOK_CASTLE_FROM[color][info];
    int rook_to = ROOK_CASTLE_TO[color][info];
    move_piece(bb, rook_from, rook_to, color, 3);
    if (color == WHITE) {
      if (info == 0)
        bb->white_queenside_rook_moved = true;
      else
        bb->white_kingside_rook_moved = true;
    } else {
      if (info == 0)
        bb->black_queenside_rook_moved = true;
      else
        bb->black_kingside_rook_moved = true;
    }
  } else if (action == PROMOTION) {
    // Remove the pawn that was just placed on 'to'
    clear_square(bb, to_bit);

    // Place the promoted piece
    if (color == WHITE) {
      bb->white_pieces |= to_mask;
      switch (promo_piece) {
      case PROMOTION_KNIGHT:
        bb->white_knights |= to_mask;
        break;
      case PROMOTION_BISHOP:
        bb->white_bishops |= to_mask;
        break;
      case PROMOTION_ROOK:
        bb->white_rooks |= to_mask;
        break;
      case PROMOTION_QUEEN:
        bb->white_queen |= to_mask;
        break;
      }
    } else {
      bb->black_pieces |= to_mask;
      switch (promo_piece) {
      case PROMOTION_KNIGHT:
        bb->black_knights |= to_mask;
        break;
      case PROMOTION_BISHOP:
        bb->black_bishops |= to_mask;
        break;
      case PROMOTION_ROOK:
        bb->black_rooks |= to_mask;
        break;
      case PROMOTION_QUEEN:
        bb->black_queen |= to_mask;
        break;
      }
    }
    bb->all_pieces = bb->white_pieces | bb->black_pieces;

    // Update game phase: pawn leaves (weight 0), promoted piece enters
    bb->game_phase += PhaseWeight[promo_piece + 1];
  } else if (action == EN_PASSANT) {
    // The captured pawn is on the square behind the destination
    int captured_sq = to_bit - 8 * flag;
    u->captured_sq = captured_sq;
    u->captured_piece = 0;
    clear_square(bb, captured_sq);
  } else if (action == DOUBLE_MOVE) {
    bb->en_passant_target = to_bit - 8 * flag;
  }

  update_eval_score(move, bb, u);

  return color;
}

void undo_move(bboard *bb, const UndoInfo *u) {
  int color = u->color;

  // 1. Reverse the special action
  if (u->action == CASTLING) {
    int info = (u->from > u->to) ? 0 : 1;
    int rook_from = ROOK_CASTLE_FROM[color][info];
    int rook_to = ROOK_CASTLE_TO[color][info];
    // Move rook back
    move_piece(bb, rook_to, rook_from, color, 3);
  }

  // 2. Move the piece back (or remove promoted piece and restore pawn)
  if (u->action == PROMOTION) {
    // Remove the promoted piece from 'to'
    clear_square(bb, u->to);
    // Place a pawn back on 'from'
    uint64_t from_mask = 1ULL << u->from;
    if (color == WHITE) {
      bb->white_pawns |= from_mask;
      bb->white_pieces |= from_mask;
    } else {
      bb->black_pawns |= from_mask;
      bb->black_pieces |= from_mask;
    }
    bb->all_pieces = bb->white_pieces | bb->black_pieces;

    bb->game_phase -= PhaseWeight[u->promo + 1];
  } else {
    // Move piece from 'to' back to 'from'
    move_piece(bb, u->to, u->from, color, u->moving_piece);
  }

  // 3. Restore captured piece
  if (u->captured_piece != -1) {
    uint64_t cap_mask = 1ULL << u->captured_sq;
    if (color == WHITE) {
      // Black piece was captured
      bb->black_pieces |= cap_mask;
      switch (u->captured_piece) {
      case 0:
        bb->black_pawns |= cap_mask;
        break;
      case 1:
        bb->black_knights |= cap_mask;
        break;
      case 2:
        bb->black_bishops |= cap_mask;
        break;
      case 3:
        bb->black_rooks |= cap_mask;
        break;
      case 4:
        bb->black_queen |= cap_mask;
        break;
      case 5:
        bb->black_king |= cap_mask;
        break;
      }
    } else {
      // White piece was captured
      bb->white_pieces |= cap_mask;
      switch (u->captured_piece) {
      case 0:
        bb->white_pawns |= cap_mask;
        break;
      case 1:
        bb->white_knights |= cap_mask;
        break;
      case 2:
        bb->white_bishops |= cap_mask;
        break;
      case 3:
        bb->white_rooks |= cap_mask;
        break;
      case 4:
        bb->white_queen |= cap_mask;
        break;
      case 5:
        bb->white_king |= cap_mask;
        break;
      }
    }
    bb->all_pieces = bb->white_pieces | bb->black_pieces;
    bb->game_phase += PhaseWeight[u->captured_piece];
  }

  // 4. Restore castling flags and en passant target
  bb->en_passant_target = u->en_passant_target;
  bb->white_king_moved = u->white_king_moved;
  bb->black_king_moved = u->black_king_moved;
  bb->white_kingside_rook_moved = u->white_kingside_rook_moved;
  bb->white_queenside_rook_moved = u->white_queenside_rook_moved;
  bb->black_kingside_rook_moved = u->black_kingside_rook_moved;
  bb->black_queenside_rook_moved = u->black_queenside_rook_moved;

  // 5. Switch turn back
  bb->turn *= -1;

  mg_score = u->old_mg_score;
  eg_score = u->old_eg_score;
}
// king_safe_after_moving
bool king_safe_after_moving(bboard *bb, int king_bit, int move) {

  int from_bit = move & FROM_MASK;
  int to_bit = (move & TO_MASK) >> 6;
  int color, piece;
  uint64_t *bb_piece = get_piece_bb(bb, from_bit, &color, &piece);
  if (!bb_piece)
    return false;
  int actual_king_bit = (piece == 5) ? to_bit : king_bit;

  UndoInfo uinfo;
  int moved_color = make_move(bb, move, &uinfo);
  int enemy_color = (moved_color == WHITE) ? BLACK : WHITE;
  bool safe = !square_attacked(bb, actual_king_bit, enemy_color);
  undo_move(bb, &uinfo);
  return safe;
}
