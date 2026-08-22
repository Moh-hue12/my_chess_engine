#ifndef STRUCTS_H
#define STRUCTS_H

#include <cpuid.h>
#include <immintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SQUARES 64

typedef struct {
  uint64_t white_pawns;
  uint64_t white_knights;
  uint64_t white_bishops;
  uint64_t white_rooks;
  uint64_t white_queen;
  uint64_t white_king;

  uint64_t black_pawns;
  uint64_t black_knights;
  uint64_t black_bishops;
  uint64_t black_rooks;
  uint64_t black_queen;
  uint64_t black_king;

  uint64_t white_pieces;
  uint64_t black_pieces;
  uint64_t all_pieces;
  bool white_king_moved;
  bool black_king_moved;
  bool white_kingside_rook_moved;
  bool white_queenside_rook_moved;
  int turn; // 1 = White to move, -1 = Black to move

  int en_passant_target; // square index of the en passant target, else -1
  bool black_kingside_rook_moved;
  bool black_queenside_rook_moved;

  int game_phase;

  int best_move;

  // Castling flags

} bboard;

#endif
