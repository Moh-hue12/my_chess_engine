# my_chess_engine

A chess engine built from scratch in C, with correctness and optimization as first priorities.

## What I Implemented

- Bitboard board representation
- Magic bitboards (PEXT-based) for sliding-piece move generation
- Legal move generation via checkers + pins + evasion mask architecture
- Alpha-beta search with MVV-LVA move ordering
- Tapered midgame/endgame evaluation with incremental updates

## Performance

- Perft depth 6 from the standard starting position: 110.9M nodes/sec (1073.67 ms)
- Correctness independently verified: node counts, captures, checks, checkmates, and discovery/double checks match known values across all six standard perft test positions

## Building and Testing

### Local Build

The project uses a Makefile that produces two executables:

- `my_engine` — the UCI chess engine
- `perft` — the perft test utility

```bash
make             # builds both executables
make run_engine  # launches the UCI engine
make run_perft   # runs the perft tests
```

### Testing with Docker

A Dockerfile is provided to run a match between this engine and Stockfish inside a portable container, using cutechess-cli.

```bash
docker build -t my_engine_test .
docker run --rm my_engine_test
```

## What to Add Next

1. Optimize the search algorithm further — add a transposition table
2. NNUE evaluation trained from self-play
