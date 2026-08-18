#include <cassert>
#include <sstream>
#include <string>

#include "chess/uci/uci.hpp"

namespace {

void test_uci_identifies_engine() {
    std::istringstream input("uci\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    std::string text = output.str();
    assert(text.find("id name devuChess") != std::string::npos);
    assert(text.find("id author devude") != std::string::npos);
    assert(text.find("uciok") != std::string::npos);
}

void test_isready_returns_readyok() {
    std::istringstream input("isready\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(output.str().find("readyok") != std::string::npos);
}

void test_go_depth_returns_bestmove() {
    std::istringstream input("position startpos\ngo depth 1\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(output.str().find("bestmove ") != std::string::npos);
}

void test_position_moves_are_applied() {
    std::istringstream input("position startpos moves e2e4 e7e5\ngo depth 1\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(output.str().find("bestmove ") != std::string::npos);
}

void test_opening_book_plays_first_move() {
    std::istringstream input("position startpos\ngo depth 1\nquit\n");
    std::ostringstream output;

    chess::run_uci_loop(input, output);

    assert(output.str().find("bestmove e2e4") != std::string::npos);
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

}

int main() {
    test_uci_identifies_engine();
    test_isready_returns_readyok();
    test_go_depth_returns_bestmove();
    test_position_moves_are_applied();
    test_opening_book_plays_first_move();
    test_invalid_depth_falls_back_to_default();
    test_go_outputs_search_info();

    return 0;
}
