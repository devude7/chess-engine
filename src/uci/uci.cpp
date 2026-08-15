#include "chess/uci/uci.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "chess/board/fen.hpp"
#include "chess/core/square.hpp"
#include "chess/search/search.hpp"
#include "chess/uci/uci_move.hpp"

namespace chess {

namespace {

constexpr const char* StartPositionFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

std::vector<std::string> split_words(const std::string& text) {
    std::istringstream stream(text);
    std::vector<std::string> words;
    std::string word;

    while (stream >> word) {
        words.push_back(word);
    }

    return words;
}

void apply_uci_moves(Board& board, const std::vector<std::string>& words, int first_move_index) {
    for (int index = first_move_index; index < static_cast<int>(words.size()); ++index) {
        Move move = move_from_uci(board, words[index]);

        if (move.from == NoSquare || move.to == NoSquare) {
            return;
        }

        UndoState undo{};
        make_move(board, move, undo);
    }
}

Board board_from_position_command(const std::string& line, const Board& current_board) {
    std::vector<std::string> words = split_words(line);

    if (words.size() < 2 || words[0] != "position") {
        return current_board;
    }

    if (words[1] == "startpos") {
        Board board = board_from_fen(StartPositionFen);

        if (words.size() > 2 && words[2] == "moves") {
            apply_uci_moves(board, words, 3);
        }

        return board;
    }

    if (words[1] == "fen") {
        if (words.size() < 8) {
            return current_board;
        }

        std::string fen = words[2] + " " + words[3] + " " + words[4] + " " + words[5] + " " + words[6] + " " + words[7];
        Board board = board_from_fen(fen);

        if (words.size() > 8 && words[8] == "moves") {
            apply_uci_moves(board, words, 9);
        }

        return board;
    }

    return current_board;
}

int depth_from_go_command(const std::string& line) {
    std::vector<std::string> words = split_words(line);

    for (int index = 0; index + 1 < static_cast<int>(words.size()); ++index) {
        if (words[index] == "depth") {
            return std::stoi(words[index + 1]);
        }
    }

    return 3;
}

}

void run_uci_loop() {
    run_uci_loop(std::cin, std::cout);
}

void run_uci_loop(std::istream& input, std::ostream& output) {
    Board board = board_from_fen(StartPositionFen);
    std::string line;

    while (std::getline(input, line)) {
        if (line == "uci") {
            output << "id name devuChess\n";
            output << "id author devude\n";
            output << "uciok\n";
            output.flush();
        } else if (line == "isready") {
            output << "readyok\n";
            output.flush();
        } else if (line == "ucinewgame") {
            board = board_from_fen(StartPositionFen);
        } else if (line.rfind("position", 0) == 0) {
            board = board_from_position_command(line, board);
        } else if (line.rfind("go", 0) == 0) {
            int depth = depth_from_go_command(line);
            SearchResult result = find_best_move(board, depth);

            output << "bestmove " << move_to_uci(result.best_move) << '\n';
            output.flush();
        } else if (line == "quit") {
            break;
        }
    }
}

}
