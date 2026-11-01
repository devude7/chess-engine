#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "chess/search/nnue.hpp"
#include "chess/uci/uci.hpp"

namespace {

void test_uci_identifies_engine() {
    std::istringstream input("uci\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    std::string text = output.str();
    assert(text.find("id name devuChess") != std::string::npos);
    assert(text.find("id author devude") != std::string::npos);
    assert(text.find("option name UseNNUE type check default false") != std::string::npos);
    assert(text.find("option name EvalFile type string default <empty>") != std::string::npos);
    assert(text.find("option name OwnBook type check default true") != std::string::npos);
    assert(text.find("option name BookFile type string default <internal>") != std::string::npos);
    assert(text.find("option name BookDepth type spin default 8 min 0 max 64") != std::string::npos);
    assert(text.find("uciok") != std::string::npos);
}

void test_setoption_toggles_nnue() {
    std::istringstream input(
        "setoption name UseNNUE value true\n"
        "setoption name UseNNUE value false\n"
        "quit\n"
    );
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    std::string text = output.str();
    assert(text.find("info string NNUE enabled") != std::string::npos);
    assert(text.find("info string NNUE disabled") != std::string::npos);
    assert(!chess::nnue_is_enabled());
}

void test_eval_reports_static_evaluation_source() {
    chess::set_nnue_enabled(false);
    chess::unload_nnue();

    std::istringstream input("position startpos\neval\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(
        output.str().find("info string eval cp 0 source classical")
        != std::string::npos
    );
}

void test_isready_returns_readyok() {
    std::istringstream input("isready\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(output.str().find("readyok") != std::string::npos);
}

void test_go_depth_returns_bestmove() {
    std::istringstream input("setoption name OwnBook value false\nposition startpos\ngo depth 1\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(output.str().find("bestmove ") != std::string::npos);
    assert(output.str().find("info depth 1") != std::string::npos);
}

void test_position_moves_are_applied() {
    std::istringstream input("position startpos moves e2e4 e7e5\ngo depth 1\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(output.str().find("bestmove ") != std::string::npos);
}

void test_invalid_depth_falls_back_to_default() {
    std::istringstream input("position fen 8/8/8/8/8/8/8/4K2k w - - 0 1\ngo depth abc\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(output.str().find("bestmove ") != std::string::npos);
}

void test_go_outputs_search_info() {
    std::istringstream input("position fen 8/8/8/8/8/8/8/4K2k w - - 0 1\ngo depth 1\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    std::string text = output.str();
    assert(text.find("info depth 1") != std::string::npos);
    assert(text.find("score cp ") != std::string::npos);
    assert(text.find("time ") != std::string::npos);
    assert(text.find("nodes ") != std::string::npos);
    assert(text.find("nps ") != std::string::npos);
    assert(text.find("tthits ") != std::string::npos);
    assert(text.find("pv ") != std::string::npos);
}

void test_go_depth_outputs_each_iterative_depth() {
    std::istringstream input("position fen 8/8/8/8/8/8/8/4K2k w - - 0 1\ngo depth 3\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    std::string text = output.str();
    assert(text.find("info depth 1") != std::string::npos);
    assert(text.find("info depth 2") != std::string::npos);
    assert(text.find("info depth 3") != std::string::npos);
    assert(text.find("bestmove ") != std::string::npos);
}

void test_go_depth_outputs_principal_variation_line() {
    std::istringstream input("position fen 8/8/8/8/8/8/8/R3K2k w Q - 0 1\ngo depth 3\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    std::string text = output.str();
    std::string marker = "info depth 3";
    std::size_t depth_position = text.find(marker);
    assert(depth_position != std::string::npos);

    std::size_t pv_position = text.find(" pv ", depth_position);
    assert(pv_position != std::string::npos);

    std::size_t line_end = text.find('\n', pv_position);
    std::string pv_line = text.substr(pv_position, line_end - pv_position);
    assert(pv_line.find(' ', 4) != std::string::npos);
}

void test_go_movetime_returns_bestmove() {
    std::istringstream input("position fen 8/8/8/8/8/8/8/4K2k w - - 0 1\ngo movetime 10\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    std::string text = output.str();
    assert(text.find("info depth ") != std::string::npos);
    assert(text.find("bestmove ") != std::string::npos);
}

void test_go_clock_time_returns_bestmove() {
    std::istringstream input("position fen 8/8/8/8/8/8/8/4K2k w - - 0 1\ngo wtime 1000 btime 1000 winc 0 binc 0\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    std::string text = output.str();
    assert(text.find("info depth ") != std::string::npos);
    assert(text.find("bestmove ") != std::string::npos);
}

void test_go_uses_book_and_newgame_keeps_it_enabled() {
    std::istringstream input(
        "position startpos\ngo depth 1\n"
        "ucinewgame\nposition startpos moves e2e4\ngo depth 1\nquit\n"
    );
    std::ostringstream output;
    chess::run_uci_loop(input, output);
    auto text = output.str();
    auto first = text.find("info string book move ");
    assert(first != std::string::npos);
    assert(text.find("info string book move ", first + 1) != std::string::npos);
    assert(text.find("info depth ") == std::string::npos);
}

void test_book_miss_and_depth_limit_use_search() {
    for (const auto& commands : {
        "position startpos moves a2a3\ngo depth 1\nquit\n",
        "setoption name BookDepth value 0\nposition startpos\ngo depth 1\nquit\n",
        "setoption name BookDepth value 1\nposition startpos moves e2e4 e7e5\ngo depth 1\nquit\n"
    }) {
        std::istringstream input(commands);
        std::ostringstream output;
        chess::run_uci_loop(input, output);
        assert(output.str().find("info string book move ") == std::string::npos);
        assert(output.str().find("info depth 1") != std::string::npos);
        assert(output.str().find("bestmove ") != std::string::npos);
    }
}

void test_book_file_with_spaces_and_restore_internal_book() {
    const char* path = "uci custom book.book";
    {
        std::ofstream file(path);
        file << "b2b3 e7e5\n";
    }
    std::istringstream input(
        "setoption name BookFile value uci custom book.book\n"
        "position startpos\ngo depth 1\n"
        "setoption name BookFile value <internal>\n"
        "position startpos moves e2e4\ngo depth 1\nquit\n"
    );
    std::ostringstream output;
    chess::run_uci_loop(input, output);
    auto text = output.str();
    assert(text.find("Opening book loaded: uci custom book.book") != std::string::npos);
    assert(text.find("bestmove b2b3") != std::string::npos);
    assert(text.find("Opening book loaded: <internal>") != std::string::npos);
    assert(text.find("info depth ") == std::string::npos);
    std::remove(path);
}

}

int main() {
    test_uci_identifies_engine();
    test_setoption_toggles_nnue();
    test_eval_reports_static_evaluation_source();
    test_isready_returns_readyok();
    test_go_depth_returns_bestmove();
    test_position_moves_are_applied();
    test_invalid_depth_falls_back_to_default();
    test_go_outputs_search_info();
    test_go_depth_outputs_each_iterative_depth();
    test_go_depth_outputs_principal_variation_line();
    test_go_movetime_returns_bestmove();
    test_go_clock_time_returns_bestmove();
    test_go_uses_book_and_newgame_keeps_it_enabled();
    test_book_miss_and_depth_limit_use_search();
    test_book_file_with_spaces_and_restore_internal_book();

    return 0;
}
