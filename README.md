# my_chess_engine

A chess engine built from scratch in C, with correctness and optimization as first priorities.

## What I Implemented

- Bitboard board representation
- Magic bitboards (PEXT-based) for sliding-piece move generation
- Legal move generation via checkers + pins + evasion mask architecture
- Alpha-beta search with MVV-LVA move ordering
- Tapered midgame/endgame evaluation with incremental updates

## Performance

- Perft depth 6 from the standard starting position: 143M nodes/sec (0.932s)
- Correctness independently verified: node counts match known values across all five standard perft test positions (Startpos, Kiwipete, Position 3, 4, 5)

## What to Add Next

1. Optimize the search algorithm further — add a hash table and transposition tables
2. NNUE evaluation trained from self-play

## Building

[add your actual build instructions here]
