#ifndef ATTACKS_H
#define ATTACKS_H

#include "movegen.h"

void init_knights_attacks(void);
void init_king_attacks(void);

bool square_attacked(const bboard *bb, int sq, int enemy_color);
uint64_t get_checkers(const bboard *bb, int sq, int enemy_color);

void pinned_pieces(const bboard *bb, int king_sq, int color);

bool move_is_legal(int from_bit, int to_bit, uint64_t evasion_mask);
void init_ray_between();

#endif
