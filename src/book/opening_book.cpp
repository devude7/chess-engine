#include "chess/book/opening_book.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

#include "chess/board/fen.hpp"
#include "chess/board/zobrist.hpp"
#include "chess/core/square.hpp"
#include "chess/default_book.hpp"
#include "chess/movegen/move_generator.hpp"
#include "chess/uci/uci_move.hpp"

namespace chess {

namespace {

constexpr const char* StartPositionFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

std::uint64_t book_key(const Board& board) {
    if (board.en_passant_square != NoSquare) {
        // FEN producers may omit an en passant square when no legal capture
        // exists. Treat those representations as the same book position.
        for (Move move : generate_legal_moves(board)) {
            if (move.type == MoveType::EnPassant) {
                return board.position_hash;
            }
        }
        return board.position_hash ^ zobrist_en_passant_key(board.en_passant_square);
    }
    return board.position_hash;
}

}

OpeningBook::OpeningBook(std::uint32_t seed) : random_(seed) {
    load_default();
}

bool OpeningBook::load(const std::string& path) {
    std::ifstream input(path);
    return input && read(input);
}

void OpeningBook::load_default() {
    std::istringstream input(DefaultOpeningBook);
    read(input);
}

bool OpeningBook::read(std::istream& input) {
    Positions positions;
    std::string line;

    while (std::getline(input, line)) {
        if (auto comment = line.find('#'); comment != std::string::npos) {
            line.erase(comment);
        }
        std::istringstream moves(line);
        Board board = board_from_fen(StartPositionFen);
        std::string text;

        while (moves >> text) {
            Move move = move_from_uci(board, text);
            if (move.from == NoSquare) {
                return false;
            }

            auto& entries = positions[book_key(board)];
            auto entry = std::find_if(entries.begin(), entries.end(), [&text](const Entry& value) {
                return value.move == text;
            });
            if (entry == entries.end()) {
                entries.push_back(Entry{text, 1.0});
            } else {
                entry->weight += 1.0;
            }

            UndoState undo{};
            if (!make_move(board, move, undo)) {
                return false;
            }
        }
    }

    if (input.bad() || positions.empty()) {
        return false;
    }

    positions_ = std::move(positions);
    return true;
}

std::optional<Move> OpeningBook::probe(const Board& board) {
    auto position = positions_.find(book_key(board));
    if (position == positions_.end()) {
        return std::nullopt;
    }

    // Revalidate candidates against this position before returning a book move.
    MoveList legal_moves = generate_legal_moves(board);
    std::vector<Move> candidates;
    std::vector<double> weights;
    for (Move move : legal_moves) {
        const std::string text = move_to_uci(move);
        for (const auto& entry : position->second) {
            if (entry.move == text) {
                candidates.push_back(move);
                weights.push_back(entry.weight);
                break;
            }
        }
    }

    if (candidates.empty()) {
        return std::nullopt;
    }

    std::discrete_distribution<std::size_t> pick(weights.begin(), weights.end());
    return candidates[pick(random_)];
}

}
