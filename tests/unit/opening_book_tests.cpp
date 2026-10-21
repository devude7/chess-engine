#include <cassert>
#include <cstdio>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include "chess/board/fen.hpp"
#include "chess/book/opening_book.hpp"
#include "chess/core/square.hpp"
#include "chess/uci/uci_move.hpp"

namespace {

constexpr const char* StartFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr const char* BookPath = "opening_book_test.book";

chess::Board after_moves(const std::string& moves) {
    chess::Board board = chess::board_from_fen(StartFen);
    std::istringstream input(moves);
    std::string text;
    while (input >> text) {
        auto move = chess::move_from_uci(board, text);
        assert(move.from != chess::NoSquare);
        chess::UndoState undo{};
        assert(chess::make_move(board, move, undo));
    }
    return board;
}

void write_book(const std::string& text) {
    std::ofstream output(BookPath);
    output << text;
}

void test_default_book_varies_first_move_and_replies_for_black() {
    chess::OpeningBook book(42);
    auto start = after_moves("");
    std::set<std::string> choices;
    for (int index = 0; index < 128; ++index) {
        auto move = book.probe(start);
        assert(move.has_value());
        choices.insert(chess::move_to_uci(*move));
    }
    assert((choices == std::set<std::string>{"e2e4", "d2d4", "c2c4"}));

    auto board = after_moves("e2e4");
    const std::set<std::string> replies{"e7e5", "c7c5", "e7e6", "c7c6"};
    auto move = book.probe(board);
    assert(move.has_value());
    assert(replies.count(chess::move_to_uci(*move)) == 1);
}

void test_fen_without_uncapturable_ep_matches_move_history() {
    chess::OpeningBook book(42);
    write_book("e2e4 c7c5\n");
    assert(book.load(BookPath));
    auto from_moves = after_moves("e2e4");
    auto from_fen = chess::board_from_fen(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
    );
    assert(chess::move_to_uci(*book.probe(from_moves)) == "c7c5");
    auto move = book.probe(from_fen);
    assert(move.has_value());
    assert(chess::move_to_uci(*move) == "c7c5");
}

void test_custom_book_miss_and_failed_reload() {
    chess::OpeningBook book(42);
    write_book("# custom opening\ne2e4 c7c5 # Sicilian\n");
    assert(book.load(BookPath));
    assert(chess::move_to_uci(*book.probe(after_moves(""))) == "e2e4");
    assert(!book.probe(after_moves("d2d4")).has_value());
    assert(!book.probe(after_moves("e2e4 c7c5")).has_value());

    for (const auto& invalid : {"e2e4 e7e5 e1e5\n", "# empty\n", "e2e5\n"}) {
        write_book(invalid);
        assert(!book.load(BookPath));
        assert(chess::move_to_uci(*book.probe(after_moves("e2e4"))) == "c7c5");
    }
    assert(!book.load("nonexistent_opening_book.book"));
    assert(chess::move_to_uci(*book.probe(after_moves("e2e4"))) == "c7c5");
    book.load_default();
    assert(book.probe(after_moves("d2d4")).has_value());
}

void test_transpositions_share_continuations() {
    chess::OpeningBook book(42);
    write_book(
        "g1f3 g8f6 b1c3 b8c6 e2e4\n"
        "b1c3 b8c6 g1f3 g8f6 d2d4\n"
    );
    assert(book.load(BookPath));
    auto board = after_moves("g1f3 g8f6 b1c3 b8c6");
    std::set<std::string> choices;
    for (int index = 0; index < 32; ++index) {
        auto move = book.probe(board);
        assert(move.has_value());
        choices.insert(chess::move_to_uci(*move));
    }
    assert((choices == std::set<std::string>{"e2e4", "d2d4"}));
}

void test_castling_and_en_passant_moves() {
    chess::OpeningBook book(42);
    auto nimzo = after_moves("d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 e2e3");
    auto castle = book.probe(nimzo);
    assert(castle.has_value());
    assert(castle->type == chess::MoveType::Castling);
    assert(chess::move_to_uci(*castle) == "e8g8");

    write_book("e2e4 a7a6 e4e5 d7d5 e5d6\n");
    assert(book.load(BookPath));
    auto board = after_moves("e2e4 a7a6 e4e5 d7d5");
    auto capture = book.probe(board);
    assert(capture.has_value());
    assert(capture->type == chess::MoveType::EnPassant);
    assert(chess::move_to_uci(*capture) == "e5d6");
}

}

int main() {
    test_default_book_varies_first_move_and_replies_for_black();
    test_fen_without_uncapturable_ep_matches_move_history();
    test_custom_book_miss_and_failed_reload();
    test_transpositions_share_continuations();
    test_castling_and_en_passant_moves();
    std::remove(BookPath);
    return 0;
}
