#include "attacks.h"
#include "movegen.h"
#include "structs.h"

uint64_t KNIGHT_ATTACKS[MAX_SQUARES];
uint64_t KING_ATTACKS[MAX_SQUARES];
uint64_t PATH_BETWEEN[MAX_SQUARES][MAX_SQUARES];
uint64_t PINNED_PIECES;
uint64_t PIN_RAYS[MAX_SQUARES];

int direction(int from, int to) {
  if (from == to)
    return 0;

  int fr = from / 8, fc = from % 8;
  int tr = to / 8, tc = to % 8;

  int dr = tr - fr;
  int dc = tc - fc;

  // Horizontal
  if (dr == 0 && dc != 0)
    return (dc > 0) ? 1 : -1;

  // Vertical
  if (dc == 0 && dr != 0)
    return (dr > 0) ? 8 : -8;

  // Diagonal
  if (abs(dr) == abs(dc))
    return (dr > 0 ? 8 : -8) + (dc > 0 ? 1 : -1);

  return 0;
}

// fill teh array PATH_BETWEEN
void init_ray_between() {
  for (int from_sq = 0; from_sq < MAX_SQUARES; from_sq++) {
    for (int to_sq = 0; to_sq < MAX_SQUARES; to_sq++) {
      uint64_t path = 0;
      int dir = direction(from_sq, to_sq);
      if (dir != 0) {
        int sq = from_sq + dir;
        while (sq != to_sq) {
          path |= (1ULL << sq);
          sq += dir;
        }
      }
      PATH_BETWEEN[from_sq][to_sq] = path;
    }
  }
}

// Knight attack initialisation
void init_knights_attacks() {
  int knight_moves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                            {1, -2},  {1, 2},  {2, -1},  {2, 1}};
  for (int sq = 0; sq < 64; sq++) {
    int r = sq / 8, c = sq % 8;
    uint64_t attacks = 0;
    for (int i = 0; i < 8; i++) {
      int rr = r + knight_moves[i][0];
      int cc = c + knight_moves[i][1];
      if (rr >= 0 && rr < 8 && cc >= 0 && cc < 8)
        attacks |= 1ULL << (rr * 8 + cc);
    }
    KNIGHT_ATTACKS[sq] = attacks;
  }
}

// King attack initialisation
void init_king_attacks() {
  int king_moves[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                          {0, 1},   {1, -1}, {1, 0},  {1, 1}};
  for (int sq = 0; sq < 64; sq++) {
    int r = sq / 8, c = sq % 8;
    uint64_t attacks = 0;
    for (int i = 0; i < 8; i++) {
      int rr = r + king_moves[i][0];
      int cc = c + king_moves[i][1];
      if (rr >= 0 && rr < 8 && cc >= 0 && cc < 8)
        attacks |= 1ULL << (rr * 8 + cc);
    }
    KING_ATTACKS[sq] = attacks;
  }
}

// Attack detection helpers (using using pre computed attacks, except for pawns)
bool enemy_pawns_can_attack(const bboard *bb, int sq, int enemy_color) {
  uint64_t pawns;
  if (enemy_color == WHITE) {
    pawns = bb->white_pawns;
    uint64_t att = ((pawns & NOT_H_FILE) << 9) | ((pawns & NOT_A_FILE) << 7);
    return (att >> sq) & 1;
  } else {
    pawns = bb->black_pawns;
    uint64_t att = ((pawns & NOT_A_FILE) >> 9) | ((pawns & NOT_H_FILE) >> 7);
    return (att >> sq) & 1;
  }
}

inline bool enemy_knight_can_attack(const bboard *bb, int sq, int enemy_color) {
  uint64_t knights =
      (enemy_color == WHITE) ? bb->white_knights : bb->black_knights;
  return (knights & KNIGHT_ATTACKS[sq]) != 0;
}

inline bool enemy_king_can_attack(const bboard *bb, int sq, int enemy_color) {
  uint64_t king = (enemy_color == WHITE) ? bb->white_king : bb->black_king;
  return (king & KING_ATTACKS[sq]) != 0;
}

/* the silding piece are symitric ,if piece a can attack piece b
 * then piece b can attack piece a, sow if we put a rook on king place and if
 * this last can reach another rook then that rook can reach the king place*/
bool enemy_can_attack_sliding(const bboard *bb, int sq, int enemy_color) {
  uint64_t occ = bb->all_pieces;

  int rook_idx = _pext_u64(bb->all_pieces, ROOK_MASKS[sq]);
  uint64_t rook_att = ROOK_ATTACKS[sq][rook_idx];

  int bishop_idx = _pext_u64(bb->all_pieces, BISHOP_MASKS[sq]);
  uint64_t bishop_att = BISHOP_ATTACKS[sq][bishop_idx];

  if (enemy_color == WHITE) {
    if (rook_att & bb->white_rooks)
      return true;
    if (bishop_att & bb->white_bishops)
      return true;
    if ((rook_att | bishop_att) & bb->white_queen)
      return true;
  } else {
    if (rook_att & bb->black_rooks)
      return true;
    if (bishop_att & bb->black_bishops)
      return true;
    if ((rook_att | bishop_att) & bb->black_queen)
      return true;
  }
  return false;
}

uint64_t check_sliding_pieces(const bboard *bb, int sq, int color) {
  uint64_t occ = bb->all_pieces;

  uint64_t attackers = 0;
  int rook_idx = _pext_u64(bb->all_pieces, ROOK_MASKS[sq]);

  uint64_t rook_att = ROOK_ATTACKS[sq][rook_idx];

  int bishop_idx = _pext_u64(bb->all_pieces, BISHOP_MASKS[sq]);

  uint64_t bishop_att = BISHOP_ATTACKS[sq][bishop_idx];

  uint64_t rooks, bishops, queen;
  if (color == BLACK) {
    rooks = bb->white_rooks;
    bishops = bb->white_bishops;
    queen = bb->white_queen;
  } else {
    rooks = bb->black_rooks;
    bishops = bb->black_bishops;
    queen = bb->black_queen;
  }

  attackers |= (rook_att & (rooks | queen));
  attackers |= (bishop_att & (bishops | queen));
  return attackers;
}

inline bool square_attacked(const bboard *bb, int sq, int enemy_color) {
  if (enemy_pawns_can_attack(bb, sq, enemy_color))
    return true;
  if (enemy_knight_can_attack(bb, sq, enemy_color))
    return true;
  if (enemy_king_can_attack(bb, sq, enemy_color))
    return true;
  if (enemy_can_attack_sliding(bb, sq, enemy_color))
    return true;
  return false;
}

// get the enemy pieces that attacks our king
uint64_t get_checkers(const bboard *bb, int sq, int color) {
  uint64_t attackers = 0;
  attackers = check_sliding_pieces(bb, sq, color);

  // check enemy king,pawn,knights
  uint64_t king, knights, pawns, pawn_att;
  if (color == BLACK) {
    king = bb->white_king;
    knights = bb->white_knights;
    pawns = bb->white_pawns;
  } else {

    king = bb->black_king;
    knights = bb->black_knights;
    pawns = bb->black_pawns;
  }

  attackers |= king & KING_ATTACKS[sq];
  attackers |= knights & KNIGHT_ATTACKS[sq];
  uint64_t pawn_attackers = 0;
  if (color == BLACK) {
    // White pawns attack north-west and north-east
    if (sq >= 9 && (sq % 8) != 0) // avoid A-file
      pawn_attackers |= pawns & (1ULL << (sq - 9));
    if (sq >= 7 && (sq % 8) != 7) // avoid H-file
      pawn_attackers |= pawns & (1ULL << (sq - 7));
  } else {
    // Black pawns attack south-west and south-east
    if (sq <= 56 && (sq % 8) != 0)
      pawn_attackers |= pawns & (1ULL << (sq + 7));
    if (sq <= 54 && (sq % 8) != 7)
      pawn_attackers |= pawns & (1ULL << (sq + 9));
  }

  attackers |= pawn_attackers;

  return attackers;
}

// get the pieces that are blocking the enmy piece from attacking ur king
void pinned_pieces(const bboard *bb, int king_sq, int color) {
  PINNED_PIECES = 0;
  memset(PIN_RAYS, 0, sizeof(PIN_RAYS));

  uint64_t own;
  uint64_t rook_like, bishop_like;
  if (color == WHITE) {
    own = bb->white_pieces;
    rook_like = bb->black_rooks | bb->black_queen;
    bishop_like = bb->black_bishops | bb->black_queen;
  } else {
    own = bb->black_pieces;
    rook_like = bb->white_rooks | bb->white_queen;
    bishop_like = bb->white_bishops | bb->white_queen;
  }

  for (int t = 0; t < 2; t++) {
    uint64_t sliding = (t == 0) ? rook_like : bishop_like;
    while (sliding) {
      uint64_t lb = sliding & -sliding;
      int slider_sq = __builtin_ctzll(lb);
      sliding &= sliding - 1;

      int dir = direction(king_sq, slider_sq);
      if (dir == 0)
        continue;

      int is_rook_dir = (dir == 8 || dir == -8 || dir == 1 || dir == -1);
      if (t == 0 && !is_rook_dir)
        continue; // rook/queen: only orthogonal
      if (t == 1 && is_rook_dir)
        continue; // bishop/queen: only diagonal

      uint64_t path = PATH_BETWEEN[king_sq][slider_sq];
      // a piece is pinned just if its th eonly piece protecting the king
      uint64_t blockers = path & bb->all_pieces;
      if (__builtin_popcountll(blockers) == 1 && (blockers & own)) {
        int pinned_sq = __builtin_ctzll(blockers);
        PINNED_PIECES |= (1ULL << pinned_sq);
        PIN_RAYS[pinned_sq] = path | (1ULL << slider_sq);
      }
    }
  }
}

// called on evry move to see if its legal or not
inline bool move_is_legal(int from_bit, int to_bit, uint64_t evasion_mask) {
  uint64_t to_bb = 1ULL << to_bit;

  // if the piece is under check protect him by capture or block
  if (evasion_mask != 0) {
    if (!(evasion_mask & to_bb)) {
      return false;
    }
  }

  // if a piece blocking the enemy piece keep it blocking
  if (PINNED_PIECES & (1ULL << from_bit)) {
    if (!(PIN_RAYS[from_bit] & to_bb)) {
      return false;
    }
  }

  return true;
}
