# chess-engine

A small C++ chess engine built as a learning project.

## Features

- FEN parsing and legal move generation, including castling, en passant, and promotions
- Make/undo moves and game state detection
- Negamax with alpha-beta pruning, iterative deepening, and quiescence search
- Transposition table, move ordering, and aspiration windows
- Classical evaluation and optional NNUE
- Built-in opening book and UCI support
- Unit and perft tests

## Build

Requires CMake and a C++17 compiler.

```powershell
cmake -S . -B build
cmake --build build
```

## Engine file

```powershell
.\build\chess_engine.exe
```

Add the executable to a UCI-compatible GUI such as Cute Chess.

## Training Data

Training data comes from [Lichess broadcast PGNs](https://database.lichess.org/#broadcasts).
`tools/nnue/generate_data.py` samples positions from games,
evaluates them with a UCI engine, and saves FEN positions and scores as JSONL.
During training, encoded positions are inputs and engine scores are targets.

Lichess publishes broadcast games under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).
Reuse, including training and commercial use, is permitted. When sharing the
source data or adaptations, credit Lichess, link the license, identify changes,
and license shared adaptations under CC BY-SA 4.0 or a compatible license.

## NNUE

Training is done in Python with PyTorch.

Default network: `773 inputs -> 256 hidden neurons (ReLU) -> 1 evaluation output`.

`tools/nnue/train.py` trains the network and saves the best validation
checkpoint. `tools/nnue/export_weights.py` extracts its weights
into a binary `.nnue` file for the C++ engine.

Enable the exported model through UCI; paths are relative to the engine's working directory:

```text
setoption name EvalFile value models/nnue/devuchess_256.nnue
setoption name UseNNUE value true
```
