import argparse
import json
import random
from pathlib import Path

import chess
import torch
from torch.utils.data import DataLoader, Dataset


PIECE_FEATURE_COUNT = 12 * 64
SIDE_TO_MOVE_INDEX = PIECE_FEATURE_COUNT
WHITE_KINGSIDE_CASTLING_INDEX = SIDE_TO_MOVE_INDEX + 1
WHITE_QUEENSIDE_CASTLING_INDEX = SIDE_TO_MOVE_INDEX + 2
BLACK_KINGSIDE_CASTLING_INDEX = SIDE_TO_MOVE_INDEX + 3
BLACK_QUEENSIDE_CASTLING_INDEX = SIDE_TO_MOVE_INDEX + 4
INPUT_SIZE = PIECE_FEATURE_COUNT + 1 + 4
HIDDEN_SIZE = 64
SCORE_SCALE = 1000.0
MAX_SCORE_CP = 1500


def encode_fen(fen):
    board = chess.Board(fen)
    features = torch.zeros(INPUT_SIZE, dtype=torch.float32)

    for square, piece in board.piece_map().items():
        color_offset = 0 if piece.color == chess.WHITE else 6
        piece_index = color_offset + piece.piece_type - 1
        feature_index = piece_index * 64 + square
        features[feature_index] = 1.0
    features[SIDE_TO_MOVE_INDEX] = (
        1.0 if board.turn == chess.WHITE else 0.0
    )
    features[WHITE_KINGSIDE_CASTLING_INDEX] = float(
        board.has_kingside_castling_rights(chess.WHITE)
    )
    features[WHITE_QUEENSIDE_CASTLING_INDEX] = float(
        board.has_queenside_castling_rights(chess.WHITE)
    )
    features[BLACK_KINGSIDE_CASTLING_INDEX] = float(
        board.has_kingside_castling_rights(chess.BLACK)
    )
    features[BLACK_QUEENSIDE_CASTLING_INDEX] = float(
        board.has_queenside_castling_rights(chess.BLACK)
    )

    return features


def normalize_score(score_cp):
    score_cp = max(-MAX_SCORE_CP, min(MAX_SCORE_CP, score_cp))
    return score_cp / SCORE_SCALE


def load_records(path):
    records = []

    with path.open(encoding='utf-8') as source:
        for line_number, line in enumerate(source, start=1):
            if not line.strip():
                continue
            try:
                record = json.loads(line)
                records.append(record)
            except json.JSONDecodeError:
                raise ValueError(f'Invalid JSON on line {line_number}')

    return records


def split_by_game(records, validation_fraction, seed):
    game_ids = sorted({record['game_id'] for record in records})

    if len(game_ids) < 2:
        raise ValueError('Dataset must contain at least two games')

    random_generator = random.Random(seed)
    random_generator.shuffle(game_ids)

    validation_count = max(1, int(len(game_ids) * validation_fraction))
    validation_count = min(validation_count, len(game_ids) - 1)

    validation_games = set(game_ids[:validation_count])

    training_records = [
        record
        for record in records
        if record["game_id"] not in validation_games
    ]

    validation_records = [
        record
        for record in records
        if record["game_id"] in validation_games
    ]

    return training_records, validation_records


class PositionDataset(Dataset):
    def __init__(self, records):
        self.records = records

    def __len__(self):
        return len(self.records)

    def __getitem__(self, index):
        record = self.records[index]

        features = encode_fen(record['fen'])
        target = normalize_score(record['score_cp'])
        target = torch.tensor([target], dtype=torch.float32)

        return features, target


class EvaluationNetwork(torch.nn.Module):
    def __init__(self):
        super().__init__()

        self.hidden_layer = torch.nn.Linear(INPUT_SIZE, HIDDEN_SIZE)
        self.output_layer = torch.nn.Linear(HIDDEN_SIZE, 1)

    def forward(self, features):
        values = self.hidden_layer(features)
        values = torch.relu(values)

        return self.output_layer(values)


def train_one_epoch(model, data_loader, loss_function, optimizer, device):
    model.train()

    total_loss = 0.0
    sample_count = 0

    for features, targets in data_loader:
        features = features.to(device)
        targets = targets.to(device)

        optimizer.zero_grad()

        predictions = model(features)
        loss = loss_function(predictions, targets)

        loss.backward()
        optimizer.step()

        batch_size = features.size(0)
        total_loss += loss.item() * batch_size
        sample_count += batch_size

    return total_loss / sample_count


def evaluate_model(model, data_loader, loss_function, device):
    model.eval()

    total_loss = 0.0
    total_error_cp = 0.0
    sample_count = 0

    with torch.no_grad():
        for features, targets in data_loader:
            features = features.to(device)
            targets = targets.to(device)

            predictions = model(features)
            loss = loss_function(predictions, targets)

            batch_size = features.size(0)
            total_loss += loss.item() * batch_size

            errors_cp = torch.abs(
                predictions - targets
            ) * SCORE_SCALE

            total_error_cp += errors_cp.sum().item()
            sample_count += batch_size

    average_loss = total_loss / sample_count
    average_error_cp = total_error_cp / sample_count

    return average_loss, average_error_cp


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("dataset", type=Path)

    parser.add_argument(
        "--output",
        type=Path,
        default=Path("models/nnue/checkpoints/model_100k_castling.pt"),
    )

    parser.add_argument("--epochs", type=int, default=20)
    parser.add_argument("--batch-size", type=int, default=256)
    parser.add_argument("--learning-rate", type=float, default=0.001)
    parser.add_argument("--validation-fraction", type=float, default=0.2)
    parser.add_argument("--patience", type=int, default=4)
    parser.add_argument("--seed", type=int, default=42)

    args = parser.parse_args()

    random.seed(args.seed)
    torch.manual_seed(args.seed)

    records = load_records(args.dataset)

    training_records, validation_records = split_by_game(
        records,
        args.validation_fraction,
        args.seed,
    )

    print(f"Training positions: {len(training_records)}")
    print(f"Validation positions: {len(validation_records)}")

    training_dataset = PositionDataset(training_records)
    validation_dataset = PositionDataset(validation_records)

    training_loader = DataLoader(
        training_dataset,
        batch_size=args.batch_size,
        shuffle=True,
    )

    validation_loader = DataLoader(
        validation_dataset,
        batch_size=args.batch_size,
        shuffle=False,
    )

    device = torch.device(
        "cuda" if torch.cuda.is_available() else "cpu"
    )

    print(f"Device: {device}")

    model = EvaluationNetwork().to(device)

    loss_function = torch.nn.SmoothL1Loss()

    optimizer = torch.optim.Adam(
        model.parameters(),
        lr=args.learning_rate,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)

    best_validation_loss = float("inf")
    best_epoch = 0
    epochs_without_improvement = 0

    for epoch in range(1, args.epochs + 1):
        training_loss = train_one_epoch(
            model,
            training_loader,
            loss_function,
            optimizer,
            device,
        )

        validation_loss, validation_error_cp = evaluate_model(
            model,
            validation_loader,
            loss_function,
            device,
        )

        print(
            f"Epoch {epoch:02d}: "
            f"train_loss={training_loss:.6f}, "
            f"validation_loss={validation_loss:.6f}, "
            f"validation_mae={validation_error_cp:.2f} cp"
        )

        if validation_loss < best_validation_loss:
            best_validation_loss = validation_loss
            best_epoch = epoch
            epochs_without_improvement = 0

            torch.save(
                {
                    "model_state_dict": model.state_dict(),
                    "input_size": INPUT_SIZE,
                    "hidden_size": HIDDEN_SIZE,
                    "score_scale": SCORE_SCALE,
                    "max_score_cp": MAX_SCORE_CP,
                    "epoch": epoch,
                    "validation_loss": validation_loss,
                    "validation_mae_cp": validation_error_cp,
                },
                args.output,
            )
        else:
            epochs_without_improvement += 1

            if epochs_without_improvement >= args.patience:
                print(
                    "Early stopping: validation loss did not improve "
                    f"for {args.patience} epochs"
                )
                break

    print(f"Best epoch: {best_epoch}")
    print(f"Best model saved to: {args.output}")


if __name__ == "__main__":
    main()
