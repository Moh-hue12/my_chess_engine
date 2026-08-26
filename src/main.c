
#include "attacks.h"
#include "bitboard.h"
#include "evaluation.h"
#include "handle_move.h"
#include "magic.h"
#include "movegen.h"
#include "search.h"

#define MAX_MOVES 256
#define MAX_INPUT 1000000
#define INF 10000
//
// static const char *square_to_string(int sq) {
//   static char buffer[3];
//   buffer[0] = 'a' + (sq % 8);
//   buffer[1] = '1' + (sq / 8);
//   buffer[2] = '\0';
//   return buffer;
// }
//
// static void move_to_uci(int move, char *uci) {
//   int from = move & FROM_MASK;
//   int to = (move & TO_MASK) >> 6;
//   int action = (move & ACTION_MASK) >> 12;
//   int promo = (move & PROMO_MASK) >> 15;
//
//   char from_str[3], to_str[3];
//   strcpy(from_str, square_to_string(from));
//   strcpy(to_str, square_to_string(to));
//
//   sprintf(uci, "%s%s", from_str, to_str);
//   if (action == PROMOTION) {
//     const char promo_pieces[] = "nbrq";
//     uci[4] = promo_pieces[promo];
//     uci[5] = '\0';
//   } else {
//     uci[4] = '\0';
//   }
// }
//
// void get_piece(int num, char *n) {
//   if (num == 0)
//     strcpy(n, "Pawn");
//   else if (num == 1)
//     strcpy(n, "Knight");
//   else if (num == 2)
//     strcpy(n, "Bishop");
//   else if (num == 3)
//     strcpy(n, "Rook");
//   else if (num == 4)
//     strcpy(n, "Queen");
//   else if (num == 5)
//     strcpy(n, "King");
//   else
//     strcpy(n, "Unknown");
// }
// int main() {
//   init_knights_attacks();
//   init_king_attacks();
//   init_ray_between();
//   init_mvv_lva_table();
//
//   bboard bb;
//   init_board(&bb);
//   init_rook_bishop_magic_bitboard(&bb);
//   init_eval_table_and_scores(&bb);
//
//   int turn = 0;
//   while (turn < 40) {
//     printf("\n========================================\n");
//     turn++;
//     printf("Turn %d\n", turn);
//
//     UndoInfo u;
//     (void)alpha_beta(&bb, 5, -INF, INF, true);
//
//     int moves[MAX_MOVES];
//     int num = 0;
//     int color = (bb.turn == 1) ? WHITE : BLACK;
//     generate_all_moves(&bb, color, moves, &num);
//
//     int legal = 0;
//     for (int i = 0; i < num; i++) {
//       if (moves[i] == bb.best_move) {
//         legal = 1;
//         break;
//       }
//     }
//
//     if (legal == 0)
//       printf("illegalmove\n");
//     else
//       printf("legal move way\n");
//
//     int move = bb.best_move;
//     int from_bit = move & FROM_MASK;
//     int to_bit = (move & TO_MASK) >> 6;
//     int action = (move & ACTION_MASK) >> 12;
//     int promo_piece = (move & PROMO_MASK) >> 15;
//
//     // --- طباعة UCI ---
//     char uci[16];
//     move_to_uci(move, uci);
//     printf("UCI: %s\n", uci);
//     printf("DEBUG: action = %d\n", action);
//
//     // --- الحصول على القطعة المصدر ---
//     int color_from, piece_from;
//     (void)get_piece_bb(&bb, from_bit, &color_from, &piece_from);
//
//     char from_name[16];
//     get_piece(piece_from, from_name);
//     printf("From: %s %s\n", (color_from == WHITE) ? "White" : "Black",
//            from_name);
//
//     // --- التحقق من الأسر (الطريقة الصحيحة) ---
//     int is_capture = 0;
//     int color_to = 0, piece_to = 0;
//
//     // افحص ما إذا كان هناك قطعة خصم في المربع الهدف
//     uint64_t enemy_pieces =
//         (bb.turn == WHITE) ? bb.black_pieces : bb.white_pieces;
//
//     if (enemy_pieces & (1ULL << to_bit)) {
//       is_capture = 1;
//       (void)get_piece_bb(&bb, to_bit, &color_to, &piece_to);
//
//       char to_name[16];
//       get_piece(piece_to, to_name);
//       printf("Captures: %s %s\n", (color_to == WHITE) ? "White" : "Black",
//              to_name);
//     }
//
//     // --- التبييت ---
//     if (action == 2) {
//       printf("Castling: O-O\n");
//     } else if (action == 3) {
//       printf("Castling: O-O-O\n");
//     }
//
//     // --- الترقية ---
//     if (action == PROMOTION && promo_piece > 0) {
//       char promo_name[16];
//       get_piece(promo_piece, promo_name);
//       printf("Promotes to: %s\n", promo_name);
//     }
//
//     // --- الأسر بالتجاوز ---
//     if (action == EN_PASSANT) {
//       printf("En Passant\n");
//     }
//
//     // --- تنفيذ النقلة ---
//     make_move(&bb, move, &u);
//     printf("turn = %d\n", bb.turn);
//     printf("white_pieces = 0x%llx\n", (unsigned long long)bb.white_pieces);
//     printf("black_pieces = 0x%llx\n", (unsigned long long)bb.black_pieces);
//     printf("black_bishops = 0x%llx\n", (unsigned long long)bb.black_bishops);
//     printf("black_king   = 0x%llx\n", (unsigned long long)bb.black_king);
//   }
//
//   return 0;
// }
int parse_move_string(char *str, bboard *bb) {
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

      if (mv_action == PROMOTION) {
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

static const char *square_to_string(int sq) {
  static char buffer[3];
  buffer[0] = 'a' + (sq % 8);
  buffer[1] = '1' + (sq / 8);
  buffer[2] = '\0';
  return buffer;
}

static void move_to_uci(int move, char *uci) {
  int from = move & FROM_MASK;
  int to = (move & TO_MASK) >> 6;
  int action = (move & ACTION_MASK) >> 12;
  int promo = (move & PROMO_MASK) >> 15;

  char from_str[3], to_str[3];
  strcpy(from_str, square_to_string(from));
  strcpy(to_str, square_to_string(to));

  sprintf(uci, "%s%s", from_str, to_str);
  if (action == PROMOTION) {
    const char promo_pieces[] = "nbrq";
    uci[4] = promo_pieces[promo];
    uci[5] = '\0';
  } else {
    uci[4] = '\0';
  }
}

// دالة لطباعة الرقعة في ملف (بدون استخدام bb->board)
void print_board_simple(FILE *log, bboard *bb) {
  // رموز القطع: البيض بالأحرف الكبيرة، السود بالأحرف الصغيرة
  char pieces[7] =
      " PNBRQK"; // 0=فارغ, 1=بيدق, 2=فرس, 3=فيل, 4=رخ, 5=وزير, 6=ملك

  fprintf(log, "  +-----------------+\n");
  for (int rank = 7; rank >= 0; rank--) {
    fprintf(log, "%d | ", rank + 1);
    for (int file = 0; file < 8; file++) {
      int sq = rank * 8 + file;
      uint64_t mask = 1ULL << sq;

      // التحقق من القطع البيضاء
      if (bb->white_pieces & mask) {
        if (bb->white_king & mask)
          fprintf(log, "K ");
        else if (bb->white_queen & mask)
          fprintf(log, "Q ");
        else if (bb->white_rooks & mask)
          fprintf(log, "R ");
        else if (bb->white_bishops & mask)
          fprintf(log, "B ");
        else if (bb->white_knights & mask)
          fprintf(log, "N ");
        else if (bb->white_pawns & mask)
          fprintf(log, "P ");
        else
          fprintf(log, "? ");
      }
      // التحقق من القطع السوداء
      else if (bb->black_pieces & mask) {
        if (bb->black_king & mask)
          fprintf(log, "k ");
        else if (bb->black_queen & mask)
          fprintf(log, "q ");
        else if (bb->black_rooks & mask)
          fprintf(log, "r ");
        else if (bb->black_bishops & mask)
          fprintf(log, "b ");
        else if (bb->black_knights & mask)
          fprintf(log, "n ");
        else if (bb->black_pawns & mask)
          fprintf(log, "p ");
        else
          fprintf(log, "? ");
      } else {
        fprintf(log, ". ");
      }
    }
    fprintf(log, "|\n");
  }
  fprintf(log, "  +-----------------+\n");
  fprintf(log, "    a b c d e f g h\n");
}

int main() {

  init_knights_attacks();
  init_king_attacks();
  init_ray_between();
  init_mvv_lva_table();

  bboard bb;
  init_board(&bb); // standard start position
  init_rook_bishop_magic_bitboard(&bb);
  init_eval_table_and_scores(&bb);

  char input[MAX_INPUT];
  while (fgets(input, MAX_INPUT, stdin)) {

    // strip both LF and CR
    input[strcspn(input, "\r\n")] = '\0';

    if (strncmp(input, "uci", 3) == 0) {
      printf("id name my_chess_engine\n");
      printf("id author Moha\n");
      printf("uciok\n");
      fflush(stdout);
    } else if (strncmp(input, "isready", 7) == 0) {
      printf("readyok\n");
      fflush(stdout);
    } else if (strncmp(input, "ucinewgame", 10) == 0) {
      init_board(&bb);
      init_eval_table_and_scores(&bb);
    } else if (strncmp(input, "position", 8) == 0) {

      char *ptr = input + 9;

      if (strncmp(ptr, "startpos", 8) == 0) {
        init_board(&bb);
        init_eval_table_and_scores(&bb);
        ptr += 8;
      }

      char *movesKeyword = strstr(ptr, "moves");
      if (movesKeyword) {

        char *token = strtok(movesKeyword + 6, " ");
        while (token) {

          int move = parse_move_string(token, &bb);
          if (move == -1) {
            FILE *log = fopen("mmmmmm.log", "a");
            if (log) {
              fprintf(log, "FAILED to parse move token: '%s'\n", token);
              fclose(log);
            }
          }
          if (move != -1) {

            UndoInfo u;
            make_move(&bb, move, &u);
          }
          token = strtok(NULL, " ");
        }
      }
    } else if (strncmp(input, "go", 2) == 0) {
      int depth = 5;
      char *token = strtok(input + 3, " ");
      while (token != NULL) {
        if (strcmp(token, "depth") == 0) {
          token = strtok(NULL, " ");
          if (token)
            depth = atoi(token);
        }
        token = strtok(NULL, " ");
      }

      bb.best_move = -1;
      int best_score = alpha_beta(&bb, depth, -INF, INF, true);

      if (bb.best_move != -1) {
        char uci_move[6];
        move_to_uci(bb.best_move, uci_move);

        printf("bestmove %s\n", uci_move);
      } else {
        printf("bestmove 0000\n");
      }
      fflush(stdout);
    } else if (strncmp(input, "quit", 4) == 0) {
      break;
    }
  }

  return 0;
}
