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

}

int main() {
    test_uci_identifies_engine();
    test_isready_returns_readyok();
    test_go_depth_returns_bestmove();
    test_position_moves_are_applied();

    return 0;
}
