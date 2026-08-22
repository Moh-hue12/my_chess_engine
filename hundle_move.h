#pragma once

#include "attacks.h"
#include "movegen.h"

typedef struct {
  int from, to;          // source and destination squares
  int action;            // NORMAL, DOUBLE_MOVE, EN_PASSANT, CASTLING, PROMOTION
  int promo;             // promotion piece index (0–3), only for PROMOTION
  int en_passant_target; // previous en-passant square
  bool white_king_moved, black_king_moved;
  bool white_kingside_rook_moved, white_queenside_rook_moved;
  bool black_kingside_rook_moved, black_queenside_rook_moved;
  int captured_piece; // piece type (0–5) if captured, else -1
  int captured_sq;    // square of captured piece
  int moving_piece;   // piece type of the moved piece
  int color;          // side that move
  int old_mg_score;
  int old_eg_score;
} UndoInfo;

uint64_t *get_piece_bb(bboard *bb, int sq, int *color, int *piece);
int make_move(bboard *bb, int move, UndoInfo *u);
void undo_move(bboard *bb, const UndoInfo *u);
bool king_safe_after_moving(bboard *bb, int king_bit, int move);
