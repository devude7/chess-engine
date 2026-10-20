#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "chess/board/fen.hpp"
#include "chess/search/evaluation.hpp"
#include "chess/search/nnue.hpp"

namespace {

constexpr int InputSize = 773;
constexpr int HiddenSize = 64;
constexpr int SideToMoveIndex = 768;
constexpr int WhiteKingsideCastlingIndex = 769;
constexpr int WhiteQueenA1Index = 4 * 64;
constexpr const char* TestModelPath = "nnue_test_weights.bin";

template <typename Value>
void write_value(std::ostream& output, const Value& value) {
    output.write(reinterpret_cast<const char*>(&value), sizeof(Value));
}

void write_values(std::ostream& output, const std::vector<float>& values) {
    output.write(
        reinterpret_cast<const char*>(values.data()),
        static_cast<std::streamsize>(values.size() * sizeof(float))
    );
}

void write_test_model(int active_feature, float feature_weight, float output_bias,
                      int neuron_count = HiddenSize, int active_neuron = 0) {
    std::ofstream output(TestModelPath, std::ios::binary);
    std::array<char, 8> magic{
        'D', 'E', 'V', 'U', 'N', 'N', 'U', 'E'
    };
    std::uint32_t version = 1;
    std::uint32_t input_size = InputSize;
    std::uint32_t hidden_size = neuron_count;
    float score_scale = 1000.0F;
    std::vector<float> hidden_weights(neuron_count * InputSize, 0.0F);
    std::vector<float> hidden_bias(neuron_count, 0.0F);
    std::vector<float> output_weights(neuron_count, 0.0F);

    if (active_feature >= 0) {
        hidden_weights[active_neuron * InputSize + active_feature] = feature_weight;
    }

    output_weights[active_neuron] = 1.0F;

    output.write(magic.data(), static_cast<std::streamsize>(magic.size()));
    write_value(output, version);
    write_value(output, input_size);
    write_value(output, hidden_size);
    write_value(output, score_scale);
    write_values(output, hidden_weights);
    write_values(output, hidden_bias);
    write_values(output, output_weights);
    write_value(output, output_bias);
}

void test_invalid_file_is_rejected() {
    std::ofstream output(TestModelPath, std::ios::binary);
    output << "invalid";
    output.close();

    chess::unload_nnue();
    assert(!chess::load_nnue(TestModelPath));
    assert(!chess::nnue_is_loaded());
}

void test_side_to_move_feature_matches_python_encoding() {
    write_test_model(SideToMoveIndex, 0.25F, 0.0F);
    assert(chess::load_nnue(TestModelPath));

    chess::Board white_to_move = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1"
    );
    chess::Board black_to_move = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/4K3 b - - 0 1"
    );

    assert(chess::evaluate_nnue(white_to_move) == 250);
    assert(chess::evaluate_nnue(black_to_move) == 0);
}

void test_castling_feature_matches_python_encoding() {
    write_test_model(WhiteKingsideCastlingIndex, 0.5F, 0.0F);
    assert(chess::load_nnue(TestModelPath));

    chess::Board with_castling = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/4K2R w K - 0 1"
    );
    chess::Board without_castling = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/4K2R w - - 0 1"
    );

    assert(chess::evaluate_nnue(with_castling) == 500);
    assert(chess::evaluate_nnue(without_castling) == 0);
}

void test_piece_square_feature_matches_python_encoding() {
    write_test_model(WhiteQueenA1Index, 0.3F, 0.0F);
    assert(chess::load_nnue(TestModelPath));

    chess::Board with_queen = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/Q3K3 w - - 0 1"
    );
    chess::Board without_queen = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1"
    );

    assert(chess::evaluate_nnue(with_queen) == 300);
    assert(chess::evaluate_nnue(without_queen) == 0);
}

void test_evaluate_uses_nnue_only_when_enabled() {
    write_test_model(-1, 0.0F, 0.125F);
    assert(chess::load_nnue(TestModelPath));

    chess::Board board = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1"
    );

    chess::set_nnue_enabled(false);
    assert(chess::evaluate(board) == chess::evaluate_classical(board));

    chess::set_nnue_enabled(true);
    assert(chess::evaluate(board) == 125);

    chess::set_nnue_enabled(false);
    chess::unload_nnue();
}

void test_wider_models_use_last_neuron() {
    chess::Board board = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/Q3K3 w - - 0 1"
    );

    for (int hidden_size : {64, 256, 512}) {
        write_test_model(WhiteQueenA1Index, 0.3F, 0.125F,
                         hidden_size, hidden_size - 1);
        assert(chess::load_nnue(TestModelPath));
        assert(chess::evaluate_nnue(board) == 425);

        write_test_model(WhiteQueenA1Index, -0.3F, 0.125F,
                         hidden_size, hidden_size - 1);
        assert(chess::load_nnue(TestModelPath));
        assert(chess::evaluate_nnue(board) == 125);
    }
    chess::unload_nnue();
}

void test_unsupported_width_preserves_loaded_model() {
    chess::Board board = chess::board_from_fen(
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1"
    );
    write_test_model(-1, 0.0F, 0.125F, 256);
    assert(chess::load_nnue(TestModelPath));

    for (std::uint32_t hidden_size : {0U, 128U, 1024U}) {
        std::fstream file(TestModelPath, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(16);
        write_value(file, hidden_size);
        file.close();

        assert(!chess::load_nnue(TestModelPath));
        assert(chess::nnue_is_loaded());
        assert(chess::evaluate_nnue(board) == 125);
    }
    chess::unload_nnue();
}

}

int main() {
    test_invalid_file_is_rejected();
    test_side_to_move_feature_matches_python_encoding();
    test_castling_feature_matches_python_encoding();
    test_piece_square_feature_matches_python_encoding();
    test_evaluate_uses_nnue_only_when_enabled();
    test_wider_models_use_last_neuron();
    test_unsupported_width_preserves_loaded_model();

    std::remove(TestModelPath);
    return 0;
}
