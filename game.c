#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attacks.h" // init_knights_attacks, init_king_attacks, init_ray_between
#include "bitboard.h" // bboard, init_board, parse_fen, etc.
#include "evaluation.h" // mg_score, eg_score, init_eval_table_and_scores, EVALUATE_CURRENT
#include "hundle_move.h" // make_move, undo_move, UndoInfo
#include "magic.h"       // init_rook_bishop_magic_bitboard
#include "movegen.h"     // generate_all_moves, move encoding masks
#include "serach.h"
#define MAX_INPUT 8192
#define MAX_MOVES 256
#define INF 1000000

// Convert square index (0..63) to "a1".."h8"
static const char *square_to_string(int sq) {
  static char buffer[3];
  buffer[0] = 'a' + (sq % 8);
  buffer[1] = '1' + (sq / 8);
  buffer[2] = '\0';
  return buffer;
}

// Convert encoded move to UCI string
static void move_to_uci(int move, char *uci) {
  int from = move & FROM_MASK;
  int to = (move & TO_MASK) >> 6;
  int action = (move & ACTION_MASK) >> 12;
  int promo = (move & PROMO_MASK) >> 15;

  sprintf(uci, "%s%s", square_to_string(from), square_to_string(to));
  if (action == 4) { // PROMOTION
    const char promo_pieces[] = "nbrq";
    uci[4] = promo_pieces[promo];
    uci[5] = '\0';
  } else {
    uci[4] = '\0';
  }
}

// Parse a move like "e2e4" or "e7e8q" and return encoded move, or -1 if
// invalid.
static int parse_move_string(const char *str, bboard *bb) {
  if (strlen(str) < 4)
    return -1;
  int from_file = str[0] - 'a';
  int from_rank = str[1] - '1';
  int to_file = str[2] - 'a';
  int to_rank = str[3] - '1';
  if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
      to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7)
    return -1;

  int from = from_rank * 8 + from_file;
  int to = to_rank * 8 + to_file;

  int moves[MAX_MOVES];
  int num = 0;
  int color = (bb->turn == 1) ? WHITE : BLACK;
  generate_all_moves(bb, color, moves, &num);

  char promo_char = (strlen(str) == 5) ? str[4] : '\0';
  const char promo_pieces[] = "nbrq";

  for (int i = 0; i < num; i++) {
    int mv = moves[i];
    int mv_from = mv & FROM_MASK;
    int mv_to = (mv & TO_MASK) >> 6;
    if (mv_from == from && mv_to == to) {
      int mv_action = (mv & ACTION_MASK) >> 12;
      int mv_promo = (mv & PROMO_MASK) >> 15;
      if (mv_action == 4) { // promotion
        if (promo_char == '\0' || promo_char == promo_pieces[mv_promo])
          return mv;
      } else {
        if (promo_char == '\0')
          return mv;
      }
    }
  }
  return -1;
}

int main() {
  char input[MAX_INPUT];
  bboard bb;

  // Initialise engine once
  init_knights_attacks();
  init_king_attacks();
  init_ray_between();
  // If your init function takes a board pointer, pass NULL or a dummy.
  // If it requires a board for all_pieces, ensure it does not use occupancy.
  init_rook_bishop_magic_bitboard(NULL);
  init_board(&bb);
  init_eval_table_and_scores(&bb);

  int search_depth = 6; // default depth

  while (fgets(input, MAX_INPUT, stdin)) {
    input[strcspn(input, "\n")] = '\0';

    if (strncmp(input, "uci", 3) == 0) {
      printf("id name MyEngine\n");
      printf("id author Student\n");
      printf("uciok\n");
      fflush(stdout);
    } else if (strncmp(input, "isready", 7) == 0) {
      printf("readyok\n");
      fflush(stdout);
    } else if (strncmp(input, "position", 8) == 0) {
      char *ptr = input + 9; // after "position "
      if (strncmp(ptr, "startpos", 8) == 0) {
        init_board(&bb);
        init_eval_table_and_scores(&bb);
        ptr += 8;
      } else if (strncmp(ptr, "fen ", 4) == 0) {
        ptr += 4;
        char fen[128];
        char *fen_end = strstr(ptr, " moves ");
        if (fen_end) {
          int len = fen_end - ptr;
          strncpy(fen, ptr, len);
          fen[len] = '\0';
          ptr = fen_end + 7;
        } else {
          strcpy(fen, ptr);
          ptr = NULL;
        }
        parse_fen(&bb, fen);
        init_eval_table_and_scores(&bb);
      }

      if (ptr != NULL) {
        char *token = strtok(ptr, " ");
        while (token != NULL) {
          int move = parse_move_string(token, &bb);
          if (move != -1) {
            UndoInfo u;
            make_move(&bb, move, &u);
            // Note: we don't undo; the board is set to that position.
          }
          token = strtok(NULL, " ");
        }
      }
    } else if (strncmp(input, "go", 2) == 0) {
      int depth = search_depth;
      char *token = strtok(input + 3, " ");
      while (token != NULL) {
        if (strcmp(token, "depth") == 0) {
          token = strtok(NULL, " ");
          if (token)
            depth = atoi(token);
        } else if (strcmp(token, "movetime") == 0) {
          token = strtok(NULL, " ");
          // We'll ignore movetime for now; you can add time management later.
        }
        token = strtok(NULL, " ");
      }

      bb.best_move = -1; // reset before root search
      int best_score = alpha_beta(&bb, depth, -INF, INF, true);

      if (bb.best_move != -1) {
        char uci_move[6];
        move_to_uci(bb.best_move, uci_move);
        printf("bestmove %s\n", uci_move);
      } else {
        printf("bestmove 0000\n"); // no legal moves (mate/stalemate)
      }
      fflush(stdout);
    } else if (strncmp(input, "quit", 4) == 0) {
      break;
    } else if (strncmp(input, "stop", 4) == 0) {
      // ignore, we don't support asynchronous stop
    }
  }

  return 0;
}
