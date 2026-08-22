// #include "attacks.h"
// #include "bitboard.h"
// #include "hundle_move.h"
// #include "magic.h"
// #include "movegen.h"
//
// #define MAX_MOVES 1000000
// PerftCounts add_counts(PerftCounts a, PerftCounts b) {
//   a.nodes += b.nodes;
//   a.normal += b.normal;
//   a.double_pawn += b.double_pawn;
//   a.castling += b.castling;
//   a.en_passant += b.en_passant;
//   a.promotions += b.promotions;
//   return a;
// }
// PerftCounts perft(bboard *bb, int depth) {
//   PerftCounts counts = {0};
//
//   int moves[MAX_MOVES];
//   int num = 0;
//   generate_all_moves(bb, bb->turn == 1 ? WHITE : BLACK, moves, &num);
//
//   if (depth == 1) {
//     for (int i = 0; i < num; i++) {
//       int from = moves[i] & FROM_MASK;
//       int to = (moves[i] & TO_MASK) >> 6;
//       int action = (moves[i] & ACTION_MASK) >> 12;
//
//       counts.nodes++;
//       switch (action) {
//       case NORMAL:
//         counts.normal++;
//         break;
//       case DOUBLE_MOVE:
//         counts.double_pawn++;
//         break;
//       case CASTLING:
//         counts.castling++;
//         break;
//       case EN_PASSANT:
//         counts.en_passant++;
//         break;
//       case PROMOTION:
//         counts.promotions++;
//         break;
//       }
//     }
//     return counts;
//   }
//
//   for (int i = 0; i < num; i++) {
//     int from = moves[i] & FROM_MASK;
//     int to = (moves[i] & TO_MASK) >> 6;
//     int action = (moves[i] & ACTION_MASK) >> 12;
//     int promo = (moves[i] & PROMO_MASK) >> 15;
//
//     bboard u;
//     make_move(bb, from, to, action, promo, &u);
//     PerftCounts child = perft(bb, depth - 1);
//     undo_move(bb, &u);
//
//     // Each child node contributes one leaf to the total,
//     // but its breakdown already counts the leaf type.
//     // So we simply add child's counts to our totals.
//     counts = add_counts(counts, child);
//   }
//
//   return counts;
// }
// // void perft_divide(bboard *bb, int depth, int color) {
// //   int moves[256];
// //   int count;
// //   generate_all_moves(bb, color, moves, &count);
// //
// //   bboard state_copy;
// //   for (int i = 0; i < count; i++) {
// //     int from = moves[i] & FROM_MASK;
// //     int to = (moves[i] & TO_MASK) >> 6;
// //     int action = (moves[i] & ACTION_MASK) >> 12;
// //     int promo = (moves[i] & PROMO_MASK) >> 15;
// //
// //     make_move(bb, from, to, action, promo, &state_copy);
// //     uint64_t nodes = perft(bb, depth - 1, (color == WHITE) ? BLACK :
// WHITE);
// //     undo_move(bb, &state_copy);
// //
// //     // Print the move in algebraic form (or at least from/to/promo)
// //     printf("%c%d%c%d", 'a' + (from % 8), 8 - from / 8, 'a' + (to % 8),
// //            8 - to / 8);
// //     if (action == PROMOTION) {
// //       const char *pieces = "nbrq"; // assuming PROMOTION_KNIGHT=0, etc.
// //       printf("%c", pieces[promo]);
// //     }
// //     printf(": %llu\n", (unsigned long long)nodes);
// //   }
// // }
// //
// int main() {
//   init_knights_attacks();
//   init_king_attacks();
//   bboard bb;
//   init_board(&bb);
//   init_rook_bishop_magic_bitboard(&bb);
//   init_ray_between();
//
//   clock_t start = clock();
//   for (int i = 1; i < 7; i++) {
//     PerftCounts res = perft(&bb, i);
//     printf("Total nodes:    %llu\n", res.nodes);
//     printf("Normal:         %llu\n", res.normal);
//     printf("Double pushes:  %llu\n", res.double_pawn);
//     printf("Castling:       %llu\n", res.castling);
//     printf("En passant:     %llu\n", res.en_passant);
//     printf("Promotions:     %llu\n", res.promotions);
//   }
//   // const char *fens[] = {
//   //     "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
//   //     "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
//   //     "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
//   //     "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
//   //     "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"};
//   // const char *names[] = {"Startpos", "Kiwipete", "Position 3", "Position
//   4",
//   //                        "Position 5"};
//   // int depths[] = {5, 4, 5, 4, 5};
//   //
//   // for (int i = 0; i < 5; i++) {
//   //   parse_fen(&bb, fens[i]);
//   //   printf("\n=== %s ===\n", names[i]);
//   //   for (int d = 1; d <= depths[i]; d++) {
//   //     uint64_t nodes = perft(&bb, d, bb.turn == 1 ? WHITE : BLACK);
//   //     printf("depth %d: %llu nodes\n", d, (unsigned long long)nodes);
//   //   }
//   // }
//   clock_t end = clock();
//   double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
//   printf("Total time: %.3f seconds\n", elapsed);
//   return 0;
// }
//
//
// #include "attacks.h"
// #include "bitboard.h" // for init_board, parse_fen etc.
// #include "hundle_move.h"
// #define MAX_MOVES 256
// #include "magic.h"
// #include "movegen.h"
// #include <stdio.h>
// #include <time.h>
//
// //
// ---------------------------------------------------------------------------
// // Breakdown counters
// //
// ---------------------------------------------------------------------------
// typedef struct {
//   uint64_t nodes;
//   uint64_t normal;
//   uint64_t double_pawn;
//   uint64_t castling;
//   uint64_t en_passant;
//   uint64_t promotions;
// } PerftCounts;
//
// static PerftCounts add_counts(PerftCounts a, PerftCounts b) {
//   a.nodes += b.nodes;
//   a.normal += b.normal;
//   a.double_pawn += b.double_pawn;
//   a.castling += b.castling;
//   a.en_passant += b.en_passant;
//   a.promotions += b.promotions;
//   return a;
// }
//
// //
// ---------------------------------------------------------------------------
// // Perft (recursive)
// //
// ---------------------------------------------------------------------------
// PerftCounts perft(bboard *bb, int depth) {
//   PerftCounts counts = {0};
//   int moves[MAX_MOVES];
//   int num = 0;
//   int color = (bb->turn == 1) ? WHITE : BLACK;
//
//   generate_all_moves(bb, color, moves, &num);
//
//   // Leaf nodes – count each move’s type once
//   if (depth == 1) {
//     for (int i = 0; i < num; i++) {
//       int action = (moves[i] & ACTION_MASK) >> 12;
//       counts.nodes++;
//       switch (action) {
//       case NORMAL:
//         counts.normal++;
//         break;
//       case DOUBLE_MOVE:
//         counts.double_pawn++;
//         break;
//       case CASTLING:
//         counts.castling++;
//         break;
//       case EN_PASSANT:
//         counts.en_passant++;
//         break;
//       case PROMOTION:
//         counts.promotions++;
//         break;
//       }
//     }
//     return counts;
//   }
//
//   // Deeper levels: make move, recurse, undo
//   for (int i = 0; i < num; i++) {
//     int from = moves[i] & FROM_MASK;
//     int to = (moves[i] & TO_MASK) >> 6;
//     int action = (moves[i] & ACTION_MASK) >> 12;
//     int promo = (moves[i] & PROMO_MASK) >> 15;
//
//     UndoInfo uinfo;
//     make_move(bb, from, to, action, promo, &uinfo);
//     PerftCounts child = perft(bb, depth - 1);
//     undo_move(bb, &uinfo);
//
//     counts = add_counts(counts, child);
//   }
//   return counts;
// }
//
// //
// ---------------------------------------------------------------------------
// // Perft divide (prints move and its subtree size)
// //
// ---------------------------------------------------------------------------
// void perft_divide(bboard *bb, int depth) {
//   int moves[MAX_MOVES];
//   int num = 0;
//   int color = (bb->turn == 1) ? WHITE : BLACK;
//   generate_all_moves(bb, color, moves, &num);
//
//   printf("Perft divide depth %d:\n", depth);
//   PerftCounts total = {0};
//
//   for (int i = 0; i < num; i++) {
//     int from = moves[i] & FROM_MASK;
//     int to = (moves[i] & TO_MASK) >> 6;
//     int action = (moves[i] & ACTION_MASK) >> 12;
//     int promo = (moves[i] & PROMO_MASK) >> 15;
//
//     UndoInfo uinfo;
//     ;
//     make_move(bb, from, to, action, promo, &uinfo);
//     PerftCounts child = perft(bb, depth - 1);
//     undo_move(bb, &uinfo);
//
//     // Print move in long algebraic (e.g. e2e4) and its node count
//     printf("%c%c%c%c: %llu\n", 'a' + (from % 8), '1' + (from / 8),
//            'a' + (to % 8), '1' + (to / 8), (unsigned long long)child.nodes);
//
//     total = add_counts(total, child);
//   }
//
//   printf("\nTotal: %llu\n", (unsigned long long)total.nodes);
//   printf("Normal:       %llu\n", total.normal);
//   printf("Double pawn:  %llu\n", total.double_pawn);
//   printf("Castling:     %llu\n", total.castling);
//   printf("En passant:   %llu\n", total.en_passant);
//   printf("Promotions:   %llu\n", total.promotions);
// }
//
// int main() {
//   // 1. Initialise static data
//   init_knights_attacks();
//   init_king_attacks();
//   init_ray_between();
//
//   bboard bb;
//   init_board(&bb); // standard start position
//   init_rook_bishop_magic_bitboard(
//       &bb); // needs the board for occupancy? It uses bb->all_pieces but may
//       not
//             // matter for init; check your implementation
//
//   clock_t start = clock();
//
//   // Run perft from depth 1 to 6 with breakdown
//   for (int depth = 1; depth <= 7; depth++) {
//     PerftCounts res = perft(&bb, depth);
//     printf("\n=== Depth %d ===\n", depth);
//     printf("Total nodes:   %llu\n", res.nodes);
//     printf("Normal:        %llu\n", res.normal);
//     printf("Double pushes: %llu\n", res.double_pawn);
//     printf("Castling:      %llu\n", res.castling);
//     printf("En passant:    %llu\n", res.en_passant);
//     printf("Promotions:    %llu\n", res.promotions);
//   }
//
//   clock_t end = clock();
//   double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
//   printf("\nTotal time: %.3f seconds\n", elapsed);
//
//   return 0;
// }
//
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "attacks.h"
#include "bitboard.h"
#include "evaluation.h"
#include "hundle_move.h"
#include "magic.h"
#include "movegen.h"
#include "serach.h"

#define MAX_MOVES 256

// ---------------------------------------------------------------------------
// Simple recursive perft – returns number of leaf nodes at the given depth.
// ---------------------------------------------------------------------------
uint64_t perft(bboard *bb, int depth) {
  int moves[MAX_MOVES];
  int num = 0;
  int color = (bb->turn == 1) ? WHITE : BLACK;

  generate_all_moves(bb, color, moves, &num);

  if (depth == 1) {
    return (uint64_t)num;
  }

  uint64_t nodes = 0;
  for (int i = 0; i < num; i++) {

    UndoInfo u;
    make_move(bb, moves[i], &u);
    nodes += perft(bb, depth - 1);
    undo_move(bb, &u);
  }
  return nodes;
}

int main() {
  // 1. Initialise static data
  init_knights_attacks();
  init_king_attacks();
  init_ray_between();
  init_mvv_lva_table();

  bboard bb;
  init_board(&bb); // standard start position
  init_rook_bishop_magic_bitboard(&bb);
  clock_t start = clock();

  // Run perft from depth 1 to 6 with breakdown
  for (int depth = 1; depth <= 6; depth++) {
    printf("depth %d : %d \n", depth, perft(&bb, depth));
  }
  alpha_beta(&bb, 20, -100000, 100000, true);
  printf("best move is : %d", bb.best_move);
  int move = bb.best_move;
  int from = move & FROM_MASK;
  int to = (move & TO_MASK) >> 6;
  int action = (move & ACTION_MASK) >> 12;
  int promo = (move & PROMO_MASK) >> 15;

  char files[] = "abcdefgh";
  printf("from: %d (%c%d) | to: %d (%c%d) | action: %d | promo: %d\n", from,
         files[from % 8], from / 8 + 1, to, files[to % 8], to / 8 + 1, action,
         promo);

  clock_t end = clock();
  double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
  printf("\nTotal time: %.3f seconds\n", elapsed);

  return 0;
}

// ---------------------------------------------------------------------------
// Main – initialise everything and run perft with timing.
// ---------------------------------------------------------------------------
// int main() {
//   // Initialise static attack tables (only once)
//   init_knights_attacks();
//   init_king_attacks();
//   init_ray_between();
//   init_rook_bishop_magic_bitboard(NULL); // or with dummy board, see note
//
//   const char *fens[] = {
//       "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
//       "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
//       "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
//       "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
//       "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"};
//   const char *names[] = {"Startpos", "Kiwipete", "Position 3", "Position
//   4",
//                          "Position 5"};
//   const int depths[] = {6, 4, 5, 4, 5}; // adjust as needed
//
//   clock_t global_start = clock();
//
//   for (int pos = 0; pos < 5; pos++) {
//     bboard bb;
//     parse_fen(&bb, fens[pos]);
//
//     // Initialize evaluation for this board
//     init_eval_table_and_scores(&bb);
//
//     printf("\n=== %s ===\n", names[pos]);
//
//     // Print initial evaluation scores
//     int phase = get_phase(&bb);
//     int full_score = (mg_score * phase + eg_score * (256 - phase)) / 256;
//     printf("mg_score: %d, eg_score: %d, phase: %d, full_score: %d\n",
//     mg_score,
//            eg_score, phase, full_score);
//
//     // Run perft for the specified depth
//     clock_t start = clock();
//     uint64_t nodes = perft(&bb, depths[pos]);
//     clock_t end = clock();
//     double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
//     printf("depth %d: %llu nodes, time %.3f s\n", depths[pos],
//            (unsigned long long)nodes, elapsed);
//
//     // Print evaluation scores after perft (should be unchanged because
//     // perft does not alter mg/eg scores, but make/undo may change
//     game_phase) phase = get_phase(&bb); full_score = (mg_score * phase +
//     eg_score * (256 - phase)) / 256; printf("After perft -> mg_score: %d,
//     eg_score: %d, full_score: %d\n",
//            mg_score, eg_score, full_score);
//   }
//
//   clock_t global_end = clock();
//   printf("\nTotal time for all positions: %.3f seconds\n",
//          (double)(global_end - global_start) / CLOCKS_PER_SEC);
//
//   return 0;
// }
