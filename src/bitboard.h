#ifndef BITBOARD_H
#define BITBOARD_H

#include "structs.h"

void init_board(bboard *bb);
void parse_fen(bboard *bb, const char *fen);

#endif
