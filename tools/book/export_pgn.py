import argparse
from pathlib import Path

import chess
import chess.pgn


def positive_int(text):
    value = int(text)
    if value <= 0:
        raise argparse.ArgumentTypeError("Value must be positive")
    return value


def main():
    parser = argparse.ArgumentParser(description="Convert PGN main lines into a UCI text opening book")
    parser.add_argument("pgn", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--max-plies", type=positive_int, default=16)
    parser.add_argument("--max-games", type=positive_int)
    args = parser.parse_args()
    if args.pgn.resolve() == args.output.resolve():
        parser.error("Input and output must be different files")

    lines = []
    with args.pgn.open(encoding="utf-8-sig") as source:
        while args.max_games is None or len(lines) < args.max_games:
            game = chess.pgn.read_game(source)
            if game is None:
                break
            if game.errors:
                parser.error(f"Invalid PGN game: {game.headers.get('Event', '?')}")
            if game.board().fen() != chess.STARTING_FEN:
                parser.error("Opening book games must start from the standard initial position")

            moves = []
            for move in game.mainline_moves():
                moves.append(move.uci())
                if len(moves) >= args.max_plies:
                    break
            if moves:
                lines.append(" ".join(moves))

    if not lines:
        parser.error("No opening lines found")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        "# UCI moves from startpos, one opening per line\n" + "\n".join(lines) + "\n",
        encoding="utf-8",
    )
    print(f"Exported {len(lines)} opening lines to: {args.output}")


if __name__ == "__main__":
    main()
