#include <cassert>
#include <cstdint>

#include "chess/board/fen.hpp"
#include "chess/board/zobrist.hpp"

namespace {

void assert_incremental_hash_is_valid(const chess::Board& board) {
    assert(board.position_hash == chess::zobrist_hash(board));
}

void test_zobrist_hash_ignores_move_counters() {
    chess::Board first = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1");
    chess::Board second = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 42 99");

    assert(chess::zobrist_hash(first) == chess::zobrist_hash(second));
}

void test_zobrist_hash_includes_side_to_move() {
    chess::Board white = chess::board_from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1");
    chess::Board black = chess::board_from_fen("8/8/8/8/8/8/8/4K2k b - - 0 1");

    assert(chess::zobrist_hash(white) != chess::zobrist_hash(black));
}

void test_zobrist_hash_includes_castling_and_en_passant() {
    chess::Board first = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    chess::Board second = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w - e3 0 1");

    assert(chess::zobrist_hash(first) != chess::zobrist_hash(second));
}

void test_zobrist_hash_is_not_empty_for_position() {
    chess::Board board = chess::board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    assert(chess::zobrist_hash(board) != std::uint64_t{0});
}

void test_board_stores_initial_zobrist_hash() {
    chess::Board board = chess::board_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    assert_incremental_hash_is_valid(board);
}

void test_incremental_hash_matches_full_hash_after_make_and_undo() {
    struct Case {
        const char* fen;
        chess::Move move;
    };

    Case cases[] = {
        Case{
            "8/8/8/3pP3/8/8/8/4K2k w - d6 0 1",
            chess::Move{36, 43, chess::MoveType::EnPassant, chess::PieceType::None}
        },
        Case{
            "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
            chess::Move{60, 62, chess::MoveType::Castling, chess::PieceType::None}
        },
        Case{
            "8/P7/8/8/8/8/8/4K2k w - - 0 1",
            chess::Move{48, 56, chess::MoveType::Promotion, chess::PieceType::Queen}
        }
    };

    for (Case test_case : cases) {
        chess::Board board = chess::board_from_fen(test_case.fen);
        std::uint64_t original_hash = board.position_hash;

        chess::UndoState undo{};
        bool ok = chess::make_move(board, test_case.move, undo);

        assert(ok);
        assert_incremental_hash_is_valid(board);

        chess::undo_move(board, test_case.move, undo);

        assert(board.position_hash == original_hash);
        assert_incremental_hash_is_valid(board);
    }
}

}

int main() {
    test_zobrist_hash_ignores_move_counters();
    test_zobrist_hash_includes_side_to_move();
    test_zobrist_hash_includes_castling_and_en_passant();
    test_zobrist_hash_is_not_empty_for_position();
    test_board_stores_initial_zobrist_hash();
    test_incremental_hash_matches_full_hash_after_make_and_undo();

    return 0;
}
