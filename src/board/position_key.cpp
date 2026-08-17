#include "chess/board/position_key.hpp"

#include "chess/core/square.hpp"

namespace chess {

namespace {

char piece_key_char(Piece piece) {
    if (piece.type == PieceType::None) {
        return '.';
    }

    char value = '.';

    switch (piece.type) {
        case PieceType::Pawn:
            value = 'p';
            break;
        case PieceType::Knight:
            value = 'n';
            break;
        case PieceType::Bishop:
            value = 'b';
            break;
        case PieceType::Rook:
            value = 'r';
            break;
        case PieceType::Queen:
            value = 'q';
            break;
        case PieceType::King:
            value = 'k';
            break;
        case PieceType::None:
            value = '.';
            break;
    }

    if (piece.color == Color::White && value != '.') {
        value = static_cast<char>(value - 'a' + 'A');
    }

    return value;
}

}

std::string position_key(const Board& board) {
    std::string key;
    key.reserve(80);

    for (Piece piece : board.squares) {
        key += piece_key_char(piece);
    }

    key += board.side_to_move == Color::White ? " w " : " b ";
    key += board.castling_rights.white_kingside ? 'K' : '-';
    key += board.castling_rights.white_queenside ? 'Q' : '-';
    key += board.castling_rights.black_kingside ? 'k' : '-';
    key += board.castling_rights.black_queenside ? 'q' : '-';
    key += ' ';

    if (board.en_passant_square == NoSquare) {
        key += '-';
    } else {
        key += static_cast<char>('a' + file_of(board.en_passant_square));
        key += static_cast<char>('1' + rank_of(board.en_passant_square));
    }

    return key;
}

}
