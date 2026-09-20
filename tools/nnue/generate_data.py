import argparse
import json
from pathlib import Path

import chess.engine
import chess.pgn


def positive_int(text):
    value = int(text)
    if value <= 0:
        raise argparse.ArgumentTypeError("Value must be positive")
    return value


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("pgn", type=Path)
    parser.add_argument("--engine", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--nodes", type=positive_int, default=10000)
    parser.add_argument("--sample-every", type=positive_int, default=4)
    parser.add_argument("--max-positions", type=positive_int)
    args = parser.parse_args()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    saved = 0
    game_id = 0

    engine = chess.engine.SimpleEngine.popen_uci(args.engine)
    try:
        with args.pgn.open(encoding="utf-8-sig") as source, \
                args.output.open("x", encoding="utf-8") as target:
            while args.max_positions is None or saved < args.max_positions:
                game = chess.pgn.read_game(source)
                if game is None:
                    break

                game_id += 1
                if game.errors:
                    continue

                board = game.board()
                for ply, move in enumerate(game.mainline_moves(), start=1):
                    board.push(move)

                    if ply % args.sample_every != 0:
                        continue
                    if board.is_game_over(claim_draw=True):
                        continue

                    info = engine.analyse(
                        board, chess.engine.Limit(nodes=args.nodes), game=game
                    )
                    score = info.get("score")
                    score_cp = None if score is None else score.white().score()
                    if score_cp is None:
                        continue

                    row = {
                        "fen": board.fen(),
                        "score_cp": score_cp,
                        "game_id": game_id,
                        "ply": ply,
                    }
                    target.write(json.dumps(row) + "\n")
                    saved += 1

                    if args.max_positions is not None and saved >= args.max_positions:
                        break
    finally:
        engine.quit()

    print(f"Saved {saved} positions from {game_id} games")


if __name__ == "__main__":
    main()
