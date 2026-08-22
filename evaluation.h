#pragma once

#include "attacks.h"
#include "bitboard.h"
#include "hundle_move.h"
#include "magic.h"
#include "movegen.h"

#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5
#define CAPTURE 5

extern int mg_score;
extern int eg_score;

extern int mg_material[6];

void init_eval_table_and_scores(const bboard *bb);
int get_phase(const bboard *bb);

void update_eval_score(int move, const bboard *bb, const UndoInfo *ui);
