#include "chess/board/fen.hpp"
#include <sstream>
#include <string>

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


int square_from_fen(const std::string& text) {
    if (text == "-") {
        return chess::NoSquare;
    }

    int file = text[0] - 'a';
    int rank = text[1] - '1';

    return rank * 8 + file;
}

} 

namespace chess {

Board board_from_fen(const std::string& fen) {
    Board board = empty_board();

    std::istringstream stream(fen);

    std::string pieces_part;
    std::string side_part;
    std::string castling_part;
    std::string en_passant_part;
    std::string halfmove_part;
    std::string fullmove_part;

    stream >> pieces_part
           >> side_part
           >> castling_part
           >> en_passant_part
           >> halfmove_part
           >> fullmove_part;

    int rank = 7;
    int file = 0;

    for (char c : pieces_part) {
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

    board.side_to_move = (side_part == "w" ? Color::White : Color::Black);

    for (char c : castling_part) {
        if (c == 'K') {
            board.castling_rights.white_kingside = true;
        } else if (c == 'Q') {
            board.castling_rights.white_queenside = true;
        } else if (c == 'k') {
            board.castling_rights.black_kingside = true;
        } else if (c == 'q') {
            board.castling_rights.black_queenside = true;
        }
    }

    board.en_passant_square = square_from_fen(en_passant_part);
    board.halfmove_clock = std::stoi(halfmove_part);
    board.fullmove_number = std::stoi(fullmove_part);

    return board;
}

}
