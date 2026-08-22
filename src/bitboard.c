// bitboard.c
#include "bitboard.h"
#include "evaluation.h"
#include <ctype.h>
#include <string.h>

void init_board(bboard *bb) {
  memset(bb, 0, sizeof(bboard));
  bb->white_pawns = 0x000000000000FF00ULL;
  bb->white_knights = 0x0000000000000042ULL;
  bb->white_bishops = 0x0000000000000024ULL;
  bb->white_rooks = 0x0000000000000081ULL;
  bb->white_queen = 0x0000000000000008ULL;
  bb->white_king = 0x0000000000000010ULL;

  bb->black_pawns = 0x00FF000000000000ULL;
  bb->black_knights = 0x4200000000000000ULL;
  bb->black_bishops = 0x2400000000000000ULL;
  bb->black_rooks = 0x8100000000000000ULL;
  bb->black_queen = 0x0800000000000000ULL;
  bb->black_king = 0x1000000000000000ULL;

  bb->white_pieces = bb->white_pawns | bb->white_knights | bb->white_bishops |
                     bb->white_rooks | bb->white_queen | bb->white_king;
  bb->black_pieces = bb->black_pawns | bb->black_knights | bb->black_bishops |
                     bb->black_rooks | bb->black_queen | bb->black_king;
  bb->all_pieces = bb->white_pieces | bb->black_pieces;

  bb->turn = 1;
  bb->en_passant_target = -1;
  bb->white_king_moved = false;
  bb->black_king_moved = false;
  bb->white_kingside_rook_moved = false;
  bb->white_queenside_rook_moved = false;
  bb->black_kingside_rook_moved = false;
  bb->black_queenside_rook_moved = false;
  bb->game_phase = get_phase(bb);
  bb->best_move = -1;
}

void parse_fen(bboard *bb, const char *fen) {
  memset(bb, 0, sizeof(bboard));

  // 1. Pieces: start at a8 (square 56)
  int sq = 56;
  const char *p = fen;
  while (*p && *p != ' ') {
    if (*p == '/') {
      sq -= 16; // next rank
      p++;
    } else if (isdigit((unsigned char)*p)) {
      sq += *p - '0'; // skip empty squares
      p++;
    } else {
      uint64_t bit = 1ULL << sq;
      switch (*p) {
      case 'P':
        bb->white_pawns |= bit;
        break;
      case 'N':
        bb->white_knights |= bit;
        break;
      case 'B':
        bb->white_bishops |= bit;
        break;
      case 'R':
        bb->white_rooks |= bit;
        break;
      case 'Q':
        bb->white_queen |= bit;
        break;
      case 'K':
        bb->white_king |= bit;
        break;
      case 'p':
        bb->black_pawns |= bit;
        break;
      case 'n':
        bb->black_knights |= bit;
        break;
      case 'b':
        bb->black_bishops |= bit;
        break;
      case 'r':
        bb->black_rooks |= bit;
        break;
      case 'q':
        bb->black_queen |= bit;
        break;
      case 'k':
        bb->black_king |= bit;
        break;
      }
      sq++;
      p++;
    }
  }

  // 2. Active color
  while (*p == ' ')
    p++;
  bb->turn = (*p == 'w') ? 1 : -1;
  p++;

  // 3. Castling rights (assume none, then enable)
  bb->white_king_moved = true;
  bb->white_kingside_rook_moved = true;
  bb->white_queenside_rook_moved = true;
  bb->black_king_moved = true;
  bb->black_kingside_rook_moved = true;
  bb->black_queenside_rook_moved = true;

  while (*p == ' ')
    p++;
  if (*p != '-') {
    while (*p && *p != ' ') {
      switch (*p) {
      case 'K':
        bb->white_king_moved = false;
        bb->white_kingside_rook_moved = false;
        break;
      case 'Q':
        bb->white_king_moved = false;
        bb->white_queenside_rook_moved = false;
        break;
      case 'k':
        bb->black_king_moved = false;
        bb->black_kingside_rook_moved = false;
        break;
      case 'q':
        bb->black_king_moved = false;
        bb->black_queenside_rook_moved = false;
        break;
      }
      p++;
    }
  } else {
    p++; // skip '-'
  }

  // 4. En passant target
  while (*p == ' ')
    p++;
  if (*p == '-' || *p == '\0') {
    bb->en_passant_target = -1;
  } else {
    int file = p[0] - 'a';
    int rank = p[1] - '1';
    bb->en_passant_target = rank * 8 + file;
    p += 2;
  }

  // 5. Aggregate bitboards
  bb->white_pieces = bb->white_pawns | bb->white_knights | bb->white_bishops |
                     bb->white_rooks | bb->white_queen | bb->white_king;
  bb->black_pieces = bb->black_pawns | bb->black_knights | bb->black_bishops |
                     bb->black_rooks | bb->black_queen | bb->black_king;
  bb->all_pieces = bb->white_pieces | bb->black_pieces;
}
