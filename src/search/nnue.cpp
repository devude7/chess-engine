#include "chess/search/nnue.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <utility>
#include <vector>

#include "chess/core/piece.hpp"

namespace chess {

namespace {

constexpr std::array<char, 8> FileMagic{
    'D', 'E', 'V', 'U', 'N', 'N', 'U', 'E'
};
constexpr std::uint32_t FormatVersion = 1;
constexpr int InputSize = 773;
constexpr int MaxHiddenSize = 512;
constexpr int SideToMoveIndex = 768;
constexpr int WhiteKingsideCastlingIndex = 769;
constexpr int WhiteQueensideCastlingIndex = 770;
constexpr int BlackKingsideCastlingIndex = 771;
constexpr int BlackQueensideCastlingIndex = 772;

struct NnueModel {
    int hidden_size = 0;
    std::vector<float> hidden_weights;
    std::vector<float> hidden_bias;
    std::vector<float> output_weights;
    float output_bias = 0.0F;
    float score_scale = 0.0F;
    bool loaded = false;
};

NnueModel Model;
bool NnueEnabled = false;

template <typename Value>
bool read_value(std::istream& input, Value& value) {
    input.read(reinterpret_cast<char*>(&value), sizeof(Value));
    return static_cast<bool>(input);
}

bool read_values(std::istream& input, std::vector<float>& values) {
    input.read(
        reinterpret_cast<char*>(values.data()),
        static_cast<std::streamsize>(values.size() * sizeof(float))
    );
    return static_cast<bool>(input);
}

bool all_finite(const std::vector<float>& values) {
    return std::all_of(values.begin(), values.end(), [](float value) {
        return std::isfinite(value);
    });
}

int feature_index(Piece piece, int square) {
    int color_offset = piece.color == Color::White ? 0 : 6;
    int piece_index = color_offset + static_cast<int>(piece.type) - 1;

    return piece_index * 64 + square;
}

void activate_feature(std::array<float, MaxHiddenSize>& hidden, int index) {
    const float* weights = Model.hidden_weights.data() + index * Model.hidden_size;
    for (int neuron = 0; neuron < Model.hidden_size; ++neuron) {
        hidden[neuron] += weights[neuron];
    }
}

}

bool load_nnue(const std::string& path) {
    std::ifstream input(path, std::ios::binary);

    if (!input) {
        return false;
    }

    std::array<char, 8> magic{};
    std::uint32_t version = 0;
    std::uint32_t input_size = 0;
    std::uint32_t hidden_size = 0;
    float score_scale = 0.0F;

    input.read(magic.data(), static_cast<std::streamsize>(magic.size()));

    if (!input
        || !read_value(input, version)
        || !read_value(input, input_size)
        || !read_value(input, hidden_size)
        || !read_value(input, score_scale)) {
        return false;
    }

    if (magic != FileMagic
        || version != FormatVersion
        || input_size != InputSize
        || (hidden_size != 64 && hidden_size != 256 && hidden_size != 512)
        || !std::isfinite(score_scale)
        || score_scale <= 0.0F) {
        return false;
    }

    NnueModel loaded_model;
    loaded_model.hidden_size = static_cast<int>(hidden_size);
    loaded_model.hidden_weights.resize(hidden_size * InputSize);
    loaded_model.hidden_bias.resize(hidden_size);
    loaded_model.output_weights.resize(hidden_size);
    loaded_model.score_scale = score_scale;

    if (!read_values(input, loaded_model.hidden_weights)
        || !read_values(input, loaded_model.hidden_bias)
        || !read_values(input, loaded_model.output_weights)
        || !read_value(input, loaded_model.output_bias)) {
        return false;
    }

    if (input.peek() != std::char_traits<char>::eof()) {
        return false;
    }

    if (!all_finite(loaded_model.hidden_weights)
        || !all_finite(loaded_model.hidden_bias)
        || !all_finite(loaded_model.output_weights)
        || !std::isfinite(loaded_model.output_bias)) {
        return false;
    }

    // Files retain the PyTorch neuron-major layout. Keep each feature's weights
    // contiguous in memory so evaluation can add one whole row at a time.
    std::vector<float> feature_weights(loaded_model.hidden_weights.size());
    for (int feature = 0; feature < InputSize; ++feature) {
        for (int neuron = 0; neuron < loaded_model.hidden_size; ++neuron) {
            feature_weights[feature * loaded_model.hidden_size + neuron] =
                loaded_model.hidden_weights[neuron * InputSize + feature];
        }
    }
    loaded_model.hidden_weights = std::move(feature_weights);

    loaded_model.loaded = true;
    Model = std::move(loaded_model);
    return true;
}

void unload_nnue() {
    Model = NnueModel{};
}

void set_nnue_enabled(bool enabled) {
    NnueEnabled = enabled;
}

bool nnue_is_enabled() {
    return NnueEnabled;
}

bool nnue_is_loaded() {
    return Model.loaded;
}

int evaluate_nnue(const Board& board) {
    if (!Model.loaded) {
        return 0;
    }

    std::array<float, MaxHiddenSize> hidden{};
    std::copy(Model.hidden_bias.begin(), Model.hidden_bias.end(), hidden.begin());

    for (int square = 0; square < 64; ++square) {
        Piece piece = board.squares[square];

        if (!is_empty(piece)) {
            activate_feature(hidden, feature_index(piece, square));
        }
    }

    if (board.side_to_move == Color::White) {
        activate_feature(hidden, SideToMoveIndex);
    }

    if (board.castling_rights.white_kingside) {
        activate_feature(hidden, WhiteKingsideCastlingIndex);
    }

    if (board.castling_rights.white_queenside) {
        activate_feature(hidden, WhiteQueensideCastlingIndex);
    }

    if (board.castling_rights.black_kingside) {
        activate_feature(hidden, BlackKingsideCastlingIndex);
    }

    if (board.castling_rights.black_queenside) {
        activate_feature(hidden, BlackQueensideCastlingIndex);
    }

    float output = Model.output_bias;

    for (int neuron = 0; neuron < Model.hidden_size; ++neuron) {
        float activation = std::max(0.0F, hidden[neuron]);
        output += activation * Model.output_weights[neuron];
    }

    return static_cast<int>(std::lround(output * Model.score_scale));
}

}
