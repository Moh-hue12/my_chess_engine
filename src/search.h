#pragma once

#include "handle_move.h"

#include "evaluation.h"

void init_mvv_lva_table();

int alpha_beta(bboard *bb, int depth, int alpha, int beta, bool is_root);
