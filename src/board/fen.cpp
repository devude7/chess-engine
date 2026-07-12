#include "chess/board/fen.hpp"

namespace {

chess::Piece piece_from_fen_char(char c) {
    using chess::Color;
    using chess::Piece;
    using chess::PieceType;

    switch (c) {
        case 'P': return Piece{PieceType::Pawn, Color::White};
        case 'N': return Piece{PieceType::Knight, Color::White};
        case 'B': return Piece{PieceType::Bishop, Color::White};
        case 'R': return Piece{PieceType::Rook, Color::White};
        case 'Q': return Piece{PieceType::Queen, Color::White};
        case 'K': return Piece{PieceType::King, Color::White};

        case 'p': return Piece{PieceType::Pawn, Color::Black};
        case 'n': return Piece{PieceType::Knight, Color::Black};
        case 'b': return Piece{PieceType::Bishop, Color::Black};
        case 'r': return Piece{PieceType::Rook, Color::Black};
        case 'q': return Piece{PieceType::Queen, Color::Black};
        case 'k': return Piece{PieceType::King, Color::Black};

        default: return Piece{PieceType::None, Color::White};
    }
}

} 

namespace chess {

Board board_from_fen(const std::string& fen) {
    Board board = empty_board();

    int rank = 7;
    int file = 0;

    for (char c : fen) {
        if (c == ' ') {
            break;
        }

        if (c == '/') {
            --rank;
            file = 0;
            continue;
        }

        if (c >= '1' && c <= '8') {
            file += c - '0';
            continue;
        }

        int square = rank * 8 + file;
        set_piece(board, square, piece_from_fen_char(c));
        ++file;
    }

    return board;
}

}
