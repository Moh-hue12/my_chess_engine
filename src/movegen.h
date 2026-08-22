#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "structs.h"
#include <stdint.h>

// ---------- bitboard masks ----------
#define NOT_A_FILE 0xFEFEFEFEFEFEFEFEULL
#define NOT_H_FILE 0x7F7F7F7F7F7F7F7FULL
#define FULL_MASK 0xFFFFFFFFFFFFFFFFULL

#define WHITE_PROMOTION_RANK 0xFF00000000000000ULL
#define BLACK_PROMOTION_RANK 0x00000000000000FFULL

// ---------- move encoding ----------
enum MoveAction { NORMAL, PROMOTION, EN_PASSANT, CASTLING, DOUBLE_MOVE };
enum PromoPiece {
  PROMOTION_KNIGHT,
  PROMOTION_BISHOP,
  PROMOTION_ROOK,
  PROMOTION_QUEEN
};
#define FROM_MASK 0x3F
#define TO_MASK (0x3F << 6)
#define ACTION_MASK (0x7 << 12)
#define PROMO_MASK (0x7 << 15)

//  colours (used in al,ost all files)
#define WHITE 0
#define BLACK 1

// sides
#define QUEENSIDE 0
#define KINGSIDE 1

// castling rook data (defined in makemove.c)
extern const int ROOK_CASTLE_FROM[2][2];
extern const int ROOK_CASTLE_TO[2][2];

// precomputed attack tables (computed in attacks.c)
#define MAX_SQUARES 64
extern uint64_t KNIGHT_ATTACKS[MAX_SQUARES];
extern uint64_t KING_ATTACKS[MAX_SQUARES];

// magic bitboards (defined in magic.c)
extern uint64_t ROOK_MAGICS[MAX_SQUARES];
extern uint64_t BISHOP_MAGICS[MAX_SQUARES];
extern uint64_t ROOK_ATTACKS[MAX_SQUARES][4096];
extern uint64_t BISHOP_ATTACKS[MAX_SQUARES][512];
extern uint64_t ROOK_MASKS[MAX_SQUARES];
extern uint64_t BISHOP_MASKS[MAX_SQUARES];

// king safty check
extern uint64_t PATH_BETWEEN[MAX_SQUARES][MAX_SQUARES];
extern uint64_t PINNED_PIECES;
extern uint64_t PIN_RAYS[MAX_SQUARES];

// move generation
void generate_all_moves(bboard *bb, int color, int *moves, int *count);

void generate_pawn_moves(bboard *bb, int color, int *moves, int *count,
                         uint64_t evasion_mask);
void generate_king_moves(bboard *bb, int color, int *moves, int *count);

void generate_knight_moves(bboard *bb, int color, int *moves, int *count,
                           uint64_t evasion_mask);

void generate_bishop_moves(bboard *bb, int color, int *moves, int *count,
                           uint64_t evasion_mask);
void generate_rook_moves(bboard *bb, int color, int *moves, int *count,
                         uint64_t evasion_mask);
void generate_queen_moves(bboard *bb, int color, int *moves, int *count,
                          uint64_t evasion_mask);

int get_king_bit(bboard *bb, int color);

// perft

#endif
