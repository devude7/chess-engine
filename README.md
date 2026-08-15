# chess-enigne

A small C++ chess engine built as a learning project. The goal is to implement the engine from the ground up, starting with correctness and then gradually improving playing strength.

## Current Features

- Board representation
- FEN parsing
- Legal move generation
- Make/undo move
- Castling, en passant, and promotions
- Perft tests
- Basic game state detection: checkmate, stalemate, result
- Material and piece-square-table evaluation
- Negamax search with alpha-beta pruning
- Basic move ordering
- Quiescence search
- Minimal UCI protocol support

## Build

```powershell
cmake --build build
```

## Run Tests

```powershell
ctest --test-dir build --output-on-failure
```

## Run Engine

```powershell
.\build\chess_engine.exe
```

Example UCI input:

```text
uci
isready
position startpos
go depth 5
quit
```

## GUI Usage

The engine can be added to a UCI-compatible chess GUI such as Cute Chess.

Engine executable:

```text
build\chess_engine.exe
```
