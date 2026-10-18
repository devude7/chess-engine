#include "chess/uci/uci.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "chess/board/fen.hpp"
#include "chess/core/square.hpp"
#include "chess/search/evaluation.hpp"
#include "chess/search/nnue.hpp"
#include "chess/search/search.hpp"
#include "chess/uci/uci_move.hpp"

namespace chess {

namespace {

constexpr const char* StartPositionFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr int DefaultSearchDepth = 10;
constexpr int MaxSearchDepth = 64;
constexpr int MinimumClockMoveTimeMs = 50;

struct GoOptions {
    int depth;
    int movetime_ms;
    bool has_movetime;
    int white_time_ms;
    int black_time_ms;
    int white_increment_ms;
    int black_increment_ms;
    bool has_clock_time;
};

struct PositionState {
    Board board;
    std::vector<std::uint64_t> position_history;
};

std::vector<std::string> split_words(const std::string& text) {
    std::istringstream stream(text);
    std::vector<std::string> words;
    std::string word;

    while (stream >> word) {
        words.push_back(word);
    }

    return words;
}

bool parse_int(const std::string& text, int& value) {
    char* end = nullptr;
    long parsed = std::strtol(text.c_str(), &end, 10);

    if (end == text.c_str() || *end != '\0') {
        return false;
    }

    value = static_cast<int>(parsed);
    return true;
}

std::string principal_variation_text(const SearchResult& result) {
    if (result.principal_variation.empty()) {
        return move_to_uci(result.best_move);
    }

    std::string text;

    for (Move move : result.principal_variation) {
        if (!text.empty()) {
            text += ' ';
        }

        text += move_to_uci(move);
    }

    return text;
}

void apply_uci_moves(PositionState& state, const std::vector<std::string>& words, int first_move_index) {
    for (int index = first_move_index; index < static_cast<int>(words.size()); ++index) {
        Move move = move_from_uci(state.board, words[index]);

        if (move.from == NoSquare || move.to == NoSquare) {
            return;
        }

        UndoState undo{};
        make_move(state.board, move, undo);
        state.position_history.push_back(state.board.position_hash);
    }
}

PositionState start_position_state() {
    PositionState state{board_from_fen(StartPositionFen), {}};
    state.position_history.push_back(state.board.position_hash);
    return state;
}

PositionState position_state_from_command(const std::string& line, const PositionState& current_state) {
    std::vector<std::string> words = split_words(line);

    if (words.size() < 2 || words[0] != "position") {
        return current_state;
    }

    if (words[1] == "startpos") {
        PositionState state = start_position_state();

        if (words.size() > 2 && words[2] == "moves") {
            apply_uci_moves(state, words, 3);
        }

        return state;
    }

    if (words[1] == "fen") {
        if (words.size() < 8) {
            return current_state;
        }

        std::string fen = words[2] + " " + words[3] + " " + words[4] + " " + words[5] + " " + words[6] + " " + words[7];
        PositionState state{board_from_fen(fen), {}};
        state.position_history.push_back(state.board.position_hash);

        if (words.size() > 8 && words[8] == "moves") {
            apply_uci_moves(state, words, 9);
        }

        return state;
    }

    return current_state;
}

GoOptions options_from_go_command(const std::string& line) {
    std::vector<std::string> words = split_words(line);
    GoOptions options{DefaultSearchDepth, 0, false, 0, 0, 0, 0, false};

    for (int index = 0; index + 1 < static_cast<int>(words.size()); ++index) {
        if (words[index] == "depth") {
            int parsed_depth = DefaultSearchDepth;

            if (parse_int(words[index + 1], parsed_depth) && parsed_depth > 0) {
                options.depth = parsed_depth;
            }
        } else if (words[index] == "movetime") {
            int parsed_movetime = 0;

            if (parse_int(words[index + 1], parsed_movetime) && parsed_movetime > 0) {
                options.movetime_ms = parsed_movetime;
                options.has_movetime = true;
                options.depth = MaxSearchDepth;
            }
        } else if (words[index] == "wtime") {
            int parsed_time = 0;

            if (parse_int(words[index + 1], parsed_time) && parsed_time > 0) {
                options.white_time_ms = parsed_time;
                options.has_clock_time = true;
            }
        } else if (words[index] == "btime") {
            int parsed_time = 0;

            if (parse_int(words[index + 1], parsed_time) && parsed_time > 0) {
                options.black_time_ms = parsed_time;
                options.has_clock_time = true;
            }
        } else if (words[index] == "winc") {
            int parsed_increment = 0;

            if (parse_int(words[index + 1], parsed_increment) && parsed_increment > 0) {
                options.white_increment_ms = parsed_increment;
            }
        } else if (words[index] == "binc") {
            int parsed_increment = 0;

            if (parse_int(words[index + 1], parsed_increment) && parsed_increment > 0) {
                options.black_increment_ms = parsed_increment;
            }
        }
    }

    return options;
}

int move_time_from_clock(const GoOptions& options, Color side_to_move) {
    int remaining_time = side_to_move == Color::White ? options.white_time_ms : options.black_time_ms;
    int increment = side_to_move == Color::White ? options.white_increment_ms : options.black_increment_ms;

    if (remaining_time <= 0) {
        return 0;
    }

    int move_time = remaining_time / 30 + increment / 2;
    int maximum_move_time = remaining_time / 3;

    if (move_time < MinimumClockMoveTimeMs && remaining_time > MinimumClockMoveTimeMs) {
        move_time = MinimumClockMoveTimeMs;
    }

    if (move_time > maximum_move_time) {
        move_time = maximum_move_time;
    }

    return move_time;
}

GoOptions apply_clock_time(const GoOptions& options, Color side_to_move) {
    if (options.has_movetime || !options.has_clock_time) {
        return options;
    }

    GoOptions updated_options = options;
    updated_options.movetime_ms = move_time_from_clock(options, side_to_move);

    if (updated_options.movetime_ms > 0) {
        updated_options.has_movetime = true;
        updated_options.depth = MaxSearchDepth;
    }

    return updated_options;
}

std::string value_from_setoption(const std::string& line, const std::string& name) {
    std::string prefix = "setoption name " + name + " value ";

    if (line.rfind(prefix, 0) != 0) {
        return "";
    }

    return line.substr(prefix.size());
}

void apply_setoption(const std::string& line, std::ostream& output) {
    std::string use_nnue = value_from_setoption(line, "UseNNUE");

    if (!use_nnue.empty()) {
        bool enabled = use_nnue == "true" || use_nnue == "1";
        set_nnue_enabled(enabled);
        output << "info string NNUE "
               << (enabled ? "enabled" : "disabled") << '\n';
        output.flush();
        return;
    }

    std::string eval_file = value_from_setoption(line, "EvalFile");

    if (!eval_file.empty()) {
        bool loaded = load_nnue(eval_file);
        output << "info string NNUE weights "
               << (loaded ? "loaded" : "failed to load")
               << ": " << eval_file << '\n';
        output.flush();
    }
}

}

void run_uci_loop() {
    run_uci_loop(std::cin, std::cout);
}

void run_uci_loop(std::istream& input, std::ostream& output) {
    PositionState state = start_position_state();
    std::string line;

    while (std::getline(input, line)) {
        if (line == "uci") {
            output << "id name devuChess\n";
            output << "id author devude\n";
            output << "option name UseNNUE type check default false\n";
            output << "option name EvalFile type string default <empty>\n";
            output << "uciok\n";
            output.flush();
        } else if (line == "isready") {
            output << "readyok\n";
            output.flush();
        } else if (line.rfind("setoption", 0) == 0) {
            apply_setoption(line, output);
        } else if (line == "ucinewgame") {
            state = start_position_state();
        } else if (line.rfind("position", 0) == 0) {
            state = position_state_from_command(line, state);
        } else if (line == "eval") {
            bool using_nnue = nnue_is_enabled() && nnue_is_loaded();
            output << "info string eval cp " << evaluate(state.board)
                   << " source " << (using_nnue ? "nnue" : "classical")
                   << '\n';
            output.flush();
        } else if (line.rfind("go", 0) == 0) {
            GoOptions go_options = apply_clock_time(options_from_go_command(line), state.board.side_to_move);
            auto search_start_time = std::chrono::steady_clock::now();
            auto search_deadline = search_start_time + std::chrono::milliseconds(go_options.movetime_ms);
            SearchResult final_result = find_best_move_iterative(
                state.board,
                go_options.depth,
                state.position_history,
                [&output, search_start_time](int current_depth, const SearchResult& result) {
                    auto current_time = std::chrono::steady_clock::now();
                    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - search_start_time).count();
                    std::uint64_t nps = elapsed_ms > 0 ? result.stats.nodes * 1000 / static_cast<std::uint64_t>(elapsed_ms) : result.stats.nodes;
                    std::string pv_text = principal_variation_text(result);

                    output << "info depth " << current_depth
                           << " score cp " << result.score
                           << " time " << elapsed_ms
                           << " nodes " << result.stats.nodes
                           << " nps " << nps
                           << " tthits " << result.stats.tt_hits
                           << " pv " << pv_text << '\n';
                    output.flush();
                },
                [go_options, search_deadline]() {
                    return go_options.has_movetime && std::chrono::steady_clock::now() >= search_deadline;
                }
            );

            output << "bestmove " << move_to_uci(final_result.best_move) << '\n';
            output.flush();
        } else if (line == "quit") {
            break;
        }
    }
}

}
