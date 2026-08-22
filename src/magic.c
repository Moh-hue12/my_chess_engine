#include "magic.h"
#include "movegen.h"

// Magic numbers(standard oriented)
uint64_t ROOK_MAGICS[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL,  0x2801880a0017001ULL,
    0x100081001000420ULL,  0x200020010080420ULL,  0x3001c0002010008ULL,
    0x8480008002000100ULL, 0x2080088004402900ULL, 0x800098204000ULL,
    0x2024401000200040ULL, 0x100802000801000ULL,  0x120800800801000ULL,
    0x208808088000400ULL,  0x2802200800400ULL,    0x2200800100020080ULL,
    0x801000060821100ULL,  0x80044006422000ULL,   0x100808020004000ULL,
    0x12108a0010204200ULL, 0x140848010000802ULL,  0x481828014002800ULL,
    0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL,  0x2040002120081000ULL, 0x21200680100081ULL,
    0x20100080080080ULL,   0x2000a00200410ULL,    0x20080800400ULL,
    0x80088400100102ULL,   0x80004600042881ULL,   0x4040008040800020ULL,
    0x440003000200801ULL,  0x4200011004500ULL,    0x188020010100100ULL,
    0x14800401802800ULL,   0x2080040080800200ULL, 0x124080204001001ULL,
    0x200046502000484ULL,  0x480400080088020ULL,  0x1000422010034000ULL,
    0x30200100110040ULL,   0x100021010009ULL,     0x2002080100110004ULL,
    0x202008004008002ULL,  0x20020004010100ULL,   0x2048440040820001ULL,
    0x101002200408200ULL,  0x40802000401080ULL,   0x4008142004410100ULL,
    0x2060820c0120200ULL,  0x1001004080100ULL,    0x20c020080040080ULL,
    0x2935610830022400ULL, 0x44440041009200ULL,   0x280001040802101ULL,
    0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL,    0x12001008414402ULL,   0x2006104900a0804ULL,
    0x1004081002402ULL};

uint64_t BISHOP_MAGICS[64] = {
    0x40040844404084ULL,   0x2004208a004208ULL,   0x10190041080202ULL,
    0x108060845042010ULL,  0x581104180800210ULL,  0x2112080446200010ULL,
    0x1080820820060210ULL, 0x3c0808410220200ULL,  0x4050404440404ULL,
    0x21001420088ULL,      0x24d0080801082102ULL, 0x1020a0a020400ULL,
    0x40308200402ULL,      0x4011002100800ULL,    0x401484104104005ULL,
    0x801010402020200ULL,  0x400210c3880100ULL,   0x404022024108200ULL,
    0x810018200204102ULL,  0x4002801a02003ULL,    0x85040820080400ULL,
    0x810102c808880400ULL, 0xe900410884800ULL,    0x8002020480840102ULL,
    0x220200865090201ULL,  0x2010100a02021202ULL, 0x152048408022401ULL,
    0x20080002081110ULL,   0x4001001021004000ULL, 0x800040400a011002ULL,
    0xe4004081011002ULL,   0x1c004001012080ULL,   0x8004200962a00220ULL,
    0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
    0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL,
    0x42008c0340209202ULL, 0x209188240001000ULL,  0x400408a884001800ULL,
    0x110400a6080400ULL,   0x1840060a44020800ULL, 0x90080104000041ULL,
    0x201011000808101ULL,  0x1a2208080504f080ULL, 0x8012020600211212ULL,
    0x500861011240000ULL,  0x180806108200800ULL,  0x4000020e01040044ULL,
    0x300000261044000aULL, 0x802241102020002ULL,  0x20906061210001ULL,
    0x5a84841004010310ULL, 0x4010801011c04ULL,    0xa010109502200ULL,
    0x4a02012000ULL,       0x500201010098b028ULL, 0x8040002811040900ULL,
    0x28000010020204ULL,   0x6000020202d0240ULL,  0x8918844842082200ULL,
    0x4010011029020020ULL};

uint64_t ROOK_MAGICS[MAX_SQUARES];
uint64_t BISHOP_MAGICS[MAX_SQUARES];
uint64_t ROOK_ATTACKS[MAX_SQUARES][4096];
uint64_t BISHOP_ATTACKS[MAX_SQUARES][512];
uint64_t ROOK_MASKS[MAX_SQUARES];
uint64_t BISHOP_MASKS[MAX_SQUARES];
void init_rook_masks(void) {
  const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  for (int sq = 0; sq < 64; sq++) {
    int r = sq / 8, c = sq % 8;
    uint64_t mask = 0;
    for (int d = 0; d < 4; d++) {
      int dr = dirs[d][0], dc = dirs[d][1];
      int row = r + dr, col = c + dc;
      while (row >= 0 && row < 8 && col >= 0 && col < 8) {
        int next_row = row + dr, next_col = col + dc;
        if (next_row < 0 || next_row >= 8 || next_col < 0 || next_col >= 8)
          break;
        mask |= 1ULL << (row * 8 + col);
        row = next_row;
        col = next_col;
      }
    }
    ROOK_MASKS[sq] = mask;
  }
}

void init_bishop_masks(void) {
  // The 4 diagonal directions
  const int dirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

  for (int sq = 0; sq < 64; sq++) {
    int r = sq / 8, c = sq % 8;
    uint64_t mask = 0;

    for (int d = 0; d < 4; d++) {
      int dr = dirs[d][0];
      int dc = dirs[d][1];
      int row = r + dr;
      int col = c + dc;

      while (row > 0 && row < 7 && col > 0 && col < 7) {
        mask |= 1ULL << (row * 8 + col);
        row += dr;
        col += dc;
      }
    }
    BISHOP_MASKS[sq] = mask;
  }
}

// walk from the piece square on all possible direction and switch when find a
// blocker
uint64_t compute_rook_attacks(int sq, uint64_t blockers) {
  uint64_t attacks = 0;
  int r = sq / 8, c = sq % 8;
  int dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  for (int i = 0; i < 4; i++) {
    int dr = dirs[i][0], dc = dirs[i][1];
    int row = r + dr, col = c + dc;
    while (row >= 0 && row < 8 && col >= 0 && col < 8) {
      int bit = row * 8 + col;
      attacks |= 1ULL << bit;
      if (blockers & (1ULL << bit))
        break;
      row += dr;
      col += dc;
    }
  }
  return attacks;
}

// walk from the piece square on all possible direction and switch when find a
// blocker
uint64_t compute_bishop_attacks(int sq, uint64_t blockers) {
  uint64_t attacks = 0;
  int r = sq / 8, c = sq % 8;
  int dirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
  for (int i = 0; i < 4; i++) {
    int dr = dirs[i][0], dc = dirs[i][1];
    int row = r + dr, col = c + dc;
    while (row >= 0 && row < 8 && col >= 0 && col < 8) {
      int bit = row * 8 + col;
      attacks |= 1ULL << bit;
      if (blockers & (1ULL << bit))
        break;
      row += dr;
      col += dc;
    }
  }
  return attacks;
}

// Count how many 1‑bits the mask has (that’s the number of squares the piece
// can reach when the board is empty).  Store those bit‑positions in an array
// so we know exactly which squares matter.  For example, if the mask has 13
// relevant squares, we’ll use a 13‑bit number to represent blockers: each bit
// corresponds to an index in the array.  If a bit is 1, the blocker sits on
// that square.  This way we can cycle through all 2^n possible blocker
// arrangements, then we genrate the possible attacks in each case.
void init_rook_attacks(const bboard *bb) {
  for (int sq = 0; sq < 64; sq++) {
    uint64_t mask = ROOK_MASKS[sq];
    int on_squares[64];
    int idx = 0;
    uint64_t temp = mask;
    while (temp) {
      uint64_t lsb = temp & -temp;
      on_squares[idx++] = __builtin_ctzll(lsb);
      temp &= temp - 1;
    }
    int possibilities = 1 << idx;
    for (int p = 0; p < possibilities; p++) {
      uint64_t blockers = 0;
      for (int i = 0; i < idx; i++) {
        if (p & (1 << i))
          blockers |= (1ULL << on_squares[i]);
      }
      uint64_t attacks = compute_rook_attacks(sq, blockers);
      int magic_idx = _pext_u64(blockers, mask);
      ROOK_ATTACKS[sq][magic_idx] = attacks;
    }
  }
}

void init_bishop_attacks(const bboard *bb) {
  for (int sq = 0; sq < 64; sq++) {
    uint64_t mask = BISHOP_MASKS[sq];
    int on_squares[64];
    int idx = 0;
    uint64_t temp = mask;
    while (temp) {
      uint64_t lsb = temp & -temp;
      on_squares[idx++] = __builtin_ctzll(lsb);
      temp &= temp - 1;
    }
    int possibilities = 1 << idx;
    for (int p = 0; p < possibilities; p++) {
      uint64_t blockers = 0;
      for (int i = 0; i < idx; i++) {
        if (p & (1 << i))
          blockers |= (1ULL << on_squares[i]);
      }
      uint64_t attacks = compute_bishop_attacks(sq, blockers);
      int magic_idx = _pext_u64(blockers, mask);
      BISHOP_ATTACKS[sq][magic_idx] = attacks;
    }
  }
}
void init_rook_bishop_magic_bitboard(const bboard *bb) {
  init_rook_masks();
  init_bishop_masks();
  init_rook_attacks(bb);
  init_bishop_attacks(bb);
}

/*
 * HOW PEXT WORKS - Step by step example PEXT (Parallel Bits Extract) takes bits
 from 'src' where 'mask' has 1s, and packs them contiguously into the result.
 * Example:
 *   src  = 0 1 0 0 0 1 1 1 1 0 0 1 0
 *   mask = 0 1 0 0 1 1 1 1 1 1 0 0 0
 *
 * Bit positions (right to left, LSB = 0):
 *   pos:  12 11 10  9  8  7  6  5  4  3  2  1  0
 *   src:   0  1  0  0  0  1  1  1  1  0  0  1  0
 *   mask:  0  1  0  0  1  1  1  1  1  1  0  0  0
 *
 * Step-by-step (iterating over set bits in mask):
 *
 *   Mask bit at pos 3:  src[3]=0  →  packed[0] = 0
 *   Mask bit at pos 4:  src[4]=1  →  packed[1] = 1
 *   Mask bit at pos 5:  src[5]=1  →  packed[2] = 1
 *   Mask bit at pos 6:  src[6]=1  →  packed[3] = 1
 *   Mask bit at pos 7:  src[7]=1  →  packed[4] = 1
 *   Mask bit at pos 8:  src[8]=0  →  packed[5] = 0
 *   Mask bit at pos 9:  src[9]=0  →  packed[6] = 0
 *   Mask bit at pos 11: src[11]=1 →  packed[7] = 1
 *
 *   mask has 8 bits set → result is 8 bits wide
 *
 * Result: 0b10011110 = 158
 *
 * Visual summary:
 *   src:  0 1 0 0 0 1 1 1 1 0 0 1 0
 *   mask: 0 1 0 0 1 1 1 1 1 1 0 0 0
 *           ↓     ↓ ↓ ↓ ↓ ↓ ↓     ↓
 *           1     0 1 1 1 1 0     0
 *           └─-───┴─┴─┴─┴─┴─┘─────┘
 *                 packed: 1 0 0 1 1 1 1 0 = 158
 *
 * Software implementation (what hardware does in 1 instruction):
 *
 *   uint64_t result = 0;
 *   int bit_pos = 0;
 *   while (mask) {
 *       int i = __builtin_ctzll(mask);      // get lowest 1-bit position
 *       if (src & (1ULL << i))              // check if source bit is set
 *           result |= (1ULL << bit_pos);    // set packed bit
 *       bit_pos++;
 *       mask &= mask - 1;                   // clear lowest 1-bit
 *   }
 *   return result;
 *

 * ===================================================================
 */
