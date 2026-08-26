#include "search.h"
#include "evaluation.h"
#include "handle_move.h"
#define MAX_MOVES 256
#define INF 1e9

#define MATE_SCORE 100000

#define EVALUATE_CURRENT(bb)                                                   \
  (((mg_score) * ((bb)->game_phase) +                                          \
    (eg_score) * (24 - ((bb)->game_phase))) /                                  \
   24)

int MVV_LVA_TABLE[6][6];

void init_mvv_lva_table() {
  for (int victim = 0; victim < 6; victim++) {
    for (int attacker = 0; attacker < 6; attacker++) {
      MVV_LVA_TABLE[victim][attacker] =
          (mg_material[victim] * 10) - mg_material[attacker];
    }
  }
}

int inline get_capt_score(bboard *bb, int move) {
  int from = move & FROM_MASK;
  int to = (move & TO_MASK) >> 6;
  int action = (move & ACTION_MASK) >> 12;
  int promo = (move & PROMO_MASK) >> 15;
  if (bb->all_pieces & (1 << to)) {
    int attacking, attacked;
    (void)get_piece_bb(bb, from, NULL, &attacking);

    (void)get_piece_bb(bb, from, NULL, &attacked);

    return MVV_LVA_TABLE[attacked][attacking];
  } else {
    return -1;
  }
}

// order the move using insertion sort
void move_ordering(bboard *bb, int *moves, int len) {
  if (len <= 1)
    return;

  int scores[MAX_MOVES];

  for (int i = 0; i < len; i++) {
    scores[i] = get_capt_score(bb, moves[i]);
  }

  for (int i = 1; i < len; i++) {
    int key_move = moves[i];
    int key_score = scores[i];
    int j = i - 1;

    while (j >= 0 && scores[j] < key_score) {
      moves[j + 1] = moves[j];
      scores[j + 1] = scores[j];
      j--;
    }

    moves[j + 1] = key_move;
    scores[j + 1] = key_score;
  }
}

int alpha_beta(bboard *bb, int depth, int alpha, int beta, bool is_root) {
  if (depth == 0) {
    return bb->turn * EVALUATE_CURRENT(bb);
  }

  int moves[MAX_MOVES];
  int num = 0;
  int color = (bb->turn == 1) ? WHITE : BLACK;

  generate_all_moves(bb, color, moves, &num);
  if (num == 0) {
    return -MATE_SCORE;
  }
  move_ordering(bb, moves, num);

  for (int i = 0; i < num; i++) {
    UndoInfo u;
    make_move(bb, moves[i], &u);

    if (is_root && num > 0 && bb->best_move == -1) {
      bb->best_move = moves[0];
    }

    int score = -alpha_beta(bb, depth - 1, -beta, -alpha, false);
    undo_move(bb, &u);

    if (score > alpha) {
      alpha = score;
      if (is_root) {
        bb->best_move = moves[i];
      }
    }

    if (!is_root && score >= beta) {
      return score; // beta cutoff
    }
  }

  return alpha;
}

//============================================================
//  ######## Other implmention of the search algorithm
//=============================================================
//
//
// int negamax(bboard *bb, int depth) {
//   if (depth == 0) {
//     return bb->turn * EVALUATE_CURRENT(bb);
//   }
//
//   int moves[MAX_MOVES];
//   int num = 0;
//   int color = (bb->turn == 1) ? WHITE : BLACK;
//
//   generate_all_moves(bb, color, moves, &num);
//
//   int best_score = -INF;
//
//   for (int i = 0; i < num; i++) {
//     UndoInfo u;
//     make_move(bb, moves[i], &u);
//
//     int score = -negamax(bb, depth - 1);
//
//     undo_move(bb, &u);
//
//     if (score > best_score) {
//       best_score = score;
//     }
//   }
//
//   return best_score;
// }
//

// int maximazer_player(bboard *bb, int depth) {
//   if (depth == 0) {
//     return EVALUATE_CURRENT(bb);
//   }
//
//   int moves[MAX_MOVES];
//   int num = 0;
//   int color = (bb->turn == 1) ? WHITE : BLACK;
//
//   generate_all_moves(bb, color, moves, &num);
//
//   int max = -INF;
//   for (int i = 0; i < num; i++) {
//     UndoInfo u;
//     make_move(bb, moves[i], &u);
//
//     int score = minimizer_player(bb, depth - 1);
//     if (score > max) {
//       max = score;
//     }
//
//     undo_move(bb, &u);
//   }
//   return max;
// }
//
// int minimizer_player(bboard *bb, int depth) {
//   if (depth == 0) {
//     return EVALUATE_CURRENT(bb);
//   }
//
//   int moves[MAX_MOVES];
//   int num = 0;
//   int color = (bb->turn == 1) ? WHITE : BLACK;
//
//   generate_all_moves(bb, color, moves, &num);
//
//   int min = INF;
//   for (int i = 0; i < num; i++) {
//     UndoInfo u;
//     make_move(bb, moves[i], &u);
//
//     int score = maximazer_player(bb, depth - 1);
//     if (score < min) {
//       min = score;
//     }
//
//     undo_move(bb, &u);
//   }
//   return min;
// }
