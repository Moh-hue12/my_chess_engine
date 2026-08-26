
#include "attacks.h"
#include "bitboard.h"
#include "hundle_move.h"
#include "magic.h"
#include "movegen.h"

#define MAX_DEPTH 6
#define MAX_MOVES 256

// Global counters (reset before each depth)
unsigned long long nodes = 0;
unsigned long long captures = 0;
unsigned long long en_passant = 0;
unsigned long long castles = 0;
unsigned long long promotions = 0;
unsigned long long checks = 0;
unsigned long long checkmates = 0;
unsigned long long discovery_checks = 0; // not used yet
unsigned long long double_checks = 0;    // not used yet

// Perft function – counts nodes and move characteristics
uint64_t perft(bboard *bb, int depth) {
  int moves[MAX_MOVES];
  int num = 0;
  int color = (bb->turn == 1) ? WHITE : BLACK;
  generate_all_moves(bb, color, moves, &num);

  if (depth == 1) {
    for (int i = 0; i < num; i++) {
      int move = moves[i];
      int from = (move & FROM_MASK);
      int to = (move & TO_MASK) >> 6;
      int action = (move & ACTION_MASK) >> 12;

      nodes++;

      uint64_t enemy_pieces =
          (color == WHITE) ? bb->black_pieces : bb->white_pieces;
      if (action == EN_PASSANT) {
        en_passant++;
        captures++;
      } else if (enemy_pieces & (1ULL << to)) {
        captures++;
      }

      if (action == PROMOTION) {
        promotions++;
      } else if (action == CASTLING) {
        castles++;
      }

      UndoInfo u;
      make_move(bb, move, &u);

      int opponent = (color == WHITE) ? BLACK : WHITE;
      int king_sq = get_king_bit(bb, opponent);
      uint64_t checkers = get_checkers(bb, king_sq, opponent);
      int num_checkers = __builtin_popcountll(checkers);

      if (num_checkers >= 1) {
        checks++;
        if (num_checkers == 2)
          double_checks++;

        // A checker counts as "the moved piece" if it sits on the primary
        // destination square OR (for castling) the rook's destination square.
        int checker_is_moved_piece = 0;
        uint64_t cbits = checkers;
        while (cbits) {
          int checker_sq = __builtin_ctzll(cbits);
          if (checker_sq == to) {
            checker_is_moved_piece = 1;
          }
          if (action == CASTLING) {
            // rook's destination depends on side/color; check both plausible
            // squares
            int rook_to = (to > from) ? ((color == WHITE) ? 5 : 61)
                                      : ((color == WHITE) ? 3 : 59);
            if (checker_sq == rook_to) {
              checker_is_moved_piece = 1;
            }
          }
          cbits &= cbits - 1;
        }
        if (!checker_is_moved_piece)
          discovery_checks++;

        int opp_moves[MAX_MOVES], opp_num = 0;
        generate_all_moves(bb, opponent, opp_moves, &opp_num);
        if (opp_num == 0)
          checkmates++;
      }
      undo_move(bb, &u);
    }
    return num;
  }

  uint64_t total = 0;
  for (int i = 0; i < num; i++) {
    UndoInfo u;
    make_move(bb, moves[i], &u);
    total += perft(bb, depth - 1);
    undo_move(bb, &u);
  }
  return total;
}

// Minimal perft for speed: only counts nodes
uint64_t perft_speed(bboard *bb, int depth) {

  int moves[MAX_MOVES];
  int num = 0;
  int color = (bb->turn == 1) ? WHITE : BLACK;
  generate_all_moves(bb, color, moves, &num);

  if (depth == 1) {

    return (uint64_t)num;
  }
  uint64_t total = 0;
  for (int i = 0; i < num; i++) {
    UndoInfo u;
    make_move(bb, moves[i], &u);
    total += perft_speed(bb, depth - 1);
    undo_move(bb, &u);
  }
  return total;
}

// ---------- Data structures for expected values ----------
typedef struct {
  long long nodes; // -1 = unknown
  long long captures;
  long long en_passant;
  long long castles;
  long long promotions;
  long long checks;
  long long discovery_checks;
  long long double_checks;
  long long checkmates;
} PerftStats;

typedef struct {
  const char *fen;
  int max_depth;
  PerftStats expected[10]; // index 0 = depth 1
} PerftTest;

static void set_stats(PerftStats *s, long long nodes, long long cap,
                      long long ep, long long cas, long long prom,
                      long long chk, long long disc, long long dbl,
                      long long mate) {
  s->nodes = nodes;
  s->captures = cap;
  s->en_passant = ep;
  s->castles = cas;
  s->promotions = prom;
  s->checks = chk;
  s->discovery_checks = disc;
  s->double_checks = dbl;
  s->checkmates = mate;
}

// ---------- Initialize test positions ----------
static void init_startpos(PerftTest *t) {
  t->fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  t->max_depth = 5;

  set_stats(&t->expected[0], 20, 0, 0, 0, 0, 0, 0, 0, 0);
  set_stats(&t->expected[1], 400, 0, 0, 0, 0, 0, 0, 0, 0);
  set_stats(&t->expected[2], 8902, 34, 0, 0, 0, 12, 0, 0, 0);
  set_stats(&t->expected[3], 197281, 1576, 0, 0, 0, 469, 0, 0, 8);
  set_stats(&t->expected[4], 4865609, 82719, 258, 0, 0, 27351, 6, 0, 347);
  set_stats(&t->expected[5], 119060324ULL, 2812008, 5248, 0, 0, 809099, 329, 46,
            10828);
}

static void init_kiwipete(PerftTest *t) {
  t->fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
  t->max_depth = 5;

  set_stats(&t->expected[0], 48, 8, 0, 2, 0, 0, 0, 0, 0);
  set_stats(&t->expected[1], 2039, 351, 1, 91, 0, 3, 0, 0, 0);
  set_stats(&t->expected[2], 97862, 17102, 45, 3162, 0, 993, 0, 0, 1);
  set_stats(&t->expected[3], 4085603, 757163, 1929, 128013, 15172, 25523, 42, 6,
            43);
  // Double checks: 2645 (corrected)
  set_stats(&t->expected[4], 193690690ULL, 35043416, 73365, 4993637, 8392,
            3309887, 19883, 2645, 30171);
  set_stats(&t->expected[5], 8031647685ULL, 1558445089ULL, 3577504, 184513607,
            56627920, 92238050, 568417, 54948, 360003);
}

static void init_pos3(PerftTest *t) {
  t->fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
  t->max_depth = 5;

  set_stats(&t->expected[0], 14, 1, 0, 0, 0, 2, 0, 0, 0);
  set_stats(&t->expected[1], 191, 14, 0, 0, 0, 10, 0, 0, 0);
  set_stats(&t->expected[2], 2812, 209, 2, 0, 0, 267, 3, 0, 0);
  set_stats(&t->expected[3], 43238, 3348, 123, 0, 0, 1680, 106, 0, 17);
  set_stats(&t->expected[4], 674624, 52051, 1165, 0, 0, 52950, 1292, 3, 0);
  set_stats(&t->expected[5], 11030083, 940350, 33325, 0, 7552, 452473, 26067, 0,
            2733);
}

static void init_pos4(PerftTest *t) {
  t->fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
  t->max_depth = 5;

  // Discovery and Double checks are NOT listed on CPW for this position;
  // set them to -1 to skip comparison.
  set_stats(&t->expected[0], 6, 0, 0, 0, 0, 0, -1, -1, 0);
  set_stats(&t->expected[1], 264, 87, 0, 6, 48, 10, -1, -1, 0);
  set_stats(&t->expected[2], 9467, 1021, 4, 0, 120, 38, -1, -1, 22);
  set_stats(&t->expected[3], 422333, 131393, 0, 7795, 60032, 15492, -1, -1, 5);
  set_stats(&t->expected[4], 15833292, 2046173, 6512, 0, 329464, 200568, -1, -1,
            50562);
  set_stats(&t->expected[5], 706045033ULL, 210369132ULL, 212, 10882006,
            81102984, 26973664, -1, -1, 81076);
}

static void init_pos5(PerftTest *t) {
  t->fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
  t->max_depth = 5;

  set_stats(&t->expected[0], 44, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[1], 1486, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[2], 62379, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[3], 2103487, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[4], 89941194, -1, -1, -1, -1, -1, -1, -1, -1);
}

static void init_pos6(PerftTest *t) {
  t->fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - "
           "- 0 10";
  t->max_depth = 5;

  set_stats(&t->expected[0], 46, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[1], 2079, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[2], 89890, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[3], 3894594, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[4], 164075551, -1, -1, -1, -1, -1, -1, -1, -1);
  set_stats(&t->expected[5], 6923051137ULL, -1, -1, -1, -1, -1, -1, -1, -1);
}

// ---------- Print helper for full test ----------
static void print_stat(const char *label, unsigned long long actual,
                       long long expected, int *mismatch) {
  printf("%-12s %12llu", label, actual);
  if (expected == -1) {
    printf("        (n/a)\n");
  } else {
    printf("  (expected %12lld)%s\n", expected,
           (actual == (unsigned long long)expected) ? "" : "  *");
    if (actual != (unsigned long long)expected)
      (*mismatch)++;
  }
}

// ---------- Full test mode ----------
static void run_full_test(bboard *bb) {
  PerftTest tests[6];
  init_startpos(&tests[0]);
  init_kiwipete(&tests[1]);
  init_pos3(&tests[2]);
  init_pos4(&tests[3]);
  init_pos5(&tests[4]);
  init_pos6(&tests[5]);

  printf("Perft comparison against CPW values\n");
  printf("Source: https://www.chessprogramming.org/Perft_Results\n\n");

  int total_mismatches = 0;

  for (int t = 0; t < 6; t++) {
    PerftTest *test = &tests[t];
    printf(
        "================================================================\n");
    printf("Position %d: %s\n", t + 1, test->fen);
    printf(
        "----------------------------------------------------------------\n");

    parse_fen(bb, test->fen);
    for (int depth = 1; depth <= test->max_depth; depth++) {
      nodes = captures = en_passant = castles = promotions = checks =
          discovery_checks = double_checks = checkmates = 0;

      uint64_t result = perft(bb, depth);

      PerftStats actual;
      actual.nodes = result;
      actual.captures = captures;
      actual.en_passant = en_passant;
      actual.castles = castles;
      actual.promotions = promotions;
      actual.checks = checks;
      actual.discovery_checks = discovery_checks;
      actual.double_checks = double_checks;
      actual.checkmates = checkmates;

      PerftStats *exp = &test->expected[depth - 1];

      printf("  Depth %d:\n", depth);
      int mism = 0;
      print_stat("Nodes:", actual.nodes, exp->nodes, &mism);
      print_stat("Captures:", actual.captures, exp->captures, &mism);
      print_stat("E.p.:", actual.en_passant, exp->en_passant, &mism);
      print_stat("Castles:", actual.castles, exp->castles, &mism);
      print_stat("Promotions:", actual.promotions, exp->promotions, &mism);
      print_stat("Checks:", actual.checks, exp->checks, &mism);
      print_stat("Discovery:", actual.discovery_checks, exp->discovery_checks,
                 &mism);
      print_stat("Double:", actual.double_checks, exp->double_checks, &mism);
      print_stat("Checkmates:", actual.checkmates, exp->checkmates, &mism);
      if (mism) {
        printf("  ** %d mismatch(es) **\n", mism);
        total_mismatches += mism;
      }
      printf("\n");
    }
  }

  printf("================================================================\n");
  printf("Total mismatches: %d\n", total_mismatches);
  if (total_mismatches == 0)
    printf("All tests passed!\n");
  else
    printf("Some tests failed.\n");
}

// ---------- Speed test mode ----------
static void run_speed_test(bboard *bb) {
  char *positions[] = {
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
      "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
      "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 "
      "10"};

  int max_depth;
  printf("Enter depth for speed test (1-6): ");
  if (scanf("%d", &max_depth) != 1 || max_depth < 1 || max_depth > 6) {
    printf("Invalid depth.\n");
    return;
  }

  printf("\nSpeed test (nodes only, depth %d):\n", max_depth);
  printf("------------------------------------------------------------\n");

  for (int i = 0; i < 6; i++) {
    parse_fen(bb, positions[i]);
    clock_t start = clock();
    uint64_t result = perft_speed(bb, max_depth);
    clock_t end = clock();
    double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

    printf("Position %d: %llu nodes, %.2f ms\n", i + 1,
           (unsigned long long)result, elapsed_ms);
  }
  printf("\n");
}

// ---------- Main menu ----------
int main() {
  // Initialize engine data
  init_knights_attacks();
  init_king_attacks();
  init_ray_between();

  bboard bb;
  init_board(&bb);
  init_rook_bishop_magic_bitboard(&bb);

  int choice;
  do {
    printf("\n");
    printf("=== Perft Test Menu ===\n");
    printf("1. Full test (compare with CPW)\n");
    printf("2. Speed test (nodes only)\n");
    printf("3. Quit\n");
    printf("Choose: ");
    if (scanf("%d", &choice) != 1) {
      printf("Invalid input, exiting.\n");
      break;
    }
    getchar(); // consume newline

    switch (choice) {
    case 1:
      run_full_test(&bb);
      break;
    case 2:
      run_speed_test(&bb);
      break;
    case 3:
      printf("Goodbye.\n");
      break;
    default:
      printf("Invalid option.\n");
    }
  } while (choice != 3);

  return 0;
}
