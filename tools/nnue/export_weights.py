import argparse
import struct
from pathlib import Path

import torch


MAGIC = b"DEVUNNUE"
FORMAT_VERSION = 1
EXPECTED_INPUT_SIZE = 773
EXPECTED_HIDDEN_SIZE = 64


def write_tensor(target, tensor):
    values = (
        tensor.detach()
        .to(device="cpu", dtype=torch.float32)
        .contiguous()
        .view(-1)
        .tolist()
    )
    target.write(struct.pack(f"<{len(values)}f", *values))


def require_shape(state_dict, name, expected_shape):
    if name not in state_dict:
        raise ValueError(f"Checkpoint does not contain {name}")

    actual_shape = tuple(state_dict[name].shape)

    if actual_shape != expected_shape:
        raise ValueError(
            f"Invalid shape for {name}: expected {expected_shape}, "
            f"got {actual_shape}"
        )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("checkpoint", type=Path)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("models/nnue/devuchess.nnue"),
    )
    args = parser.parse_args()

    checkpoint = torch.load(
        args.checkpoint,
        map_location="cpu",
        weights_only=True,
    )

    input_size = checkpoint.get("input_size")
    hidden_size = checkpoint.get("hidden_size")
    score_scale = checkpoint.get("score_scale")

    if input_size != EXPECTED_INPUT_SIZE:
        raise ValueError(
            f"Expected input size {EXPECTED_INPUT_SIZE}, got {input_size}"
        )

    if hidden_size != EXPECTED_HIDDEN_SIZE:
        raise ValueError(
            f"Expected hidden size {EXPECTED_HIDDEN_SIZE}, got {hidden_size}"
        )

    if score_scale is None or score_scale <= 0:
        raise ValueError("Checkpoint has an invalid score scale")

    state_dict = checkpoint["model_state_dict"]
    require_shape(
        state_dict,
        "hidden_layer.weight",
        (hidden_size, input_size),
    )
    require_shape(
        state_dict,
        "hidden_layer.bias",
        (hidden_size,),
    )
    require_shape(
        state_dict,
        "output_layer.weight",
        (1, hidden_size),
    )
    require_shape(state_dict, "output_layer.bias", (1,))

    args.output.parent.mkdir(parents=True, exist_ok=True)

    with args.output.open("wb") as target:
        target.write(
            struct.pack(
                "<8sIIIf",
                MAGIC,
                FORMAT_VERSION,
                input_size,
                hidden_size,
                score_scale,
            )
        )
        write_tensor(target, state_dict["hidden_layer.weight"])
        write_tensor(target, state_dict["hidden_layer.bias"])
        write_tensor(target, state_dict["output_layer.weight"])
        write_tensor(target, state_dict["output_layer.bias"])

    print(f"Exported NNUE weights to: {args.output}")


if __name__ == "__main__":
    main()
