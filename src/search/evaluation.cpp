#include "chess/search/evaluation.hpp"

#include "chess/core/square.hpp"

namespace chess {

namespace {

constexpr int PawnTable[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     10,  10,  10, -10, -10,  10,  10,  10,
      5,   5,  10,  20,  20,  10,   5,   5,
      0,   0,   0,  25,  25,   0,   0,   0,
      5,   5,  10,  30,  30,  10,   5,   5,
     10,  10,  20,  35,  35,  20,  10,  10,
     50,  50,  50,  50,  50,  50,  50,  50,
      0,   0,   0,   0,   0,   0,   0,   0
};

constexpr int KnightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

constexpr int BishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   0,  10,  15,  15,  10,   0, -10,
    -10,   5,   5,  15,  15,   5,   5, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

constexpr int RookTable[64] = {
      0,   0,   5,  10,  10,   5,   0,   0,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
      5,  10,  10,  10,  10,  10,  10,   5,
      0,   0,   5,  10,  10,   5,   0,   0
};

constexpr int QueenTable[64] = {
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -10,   5,   5,   5,   5,   5,   0, -10,
      0,   0,   5,   5,   5,   5,   0,  -5,
     -5,   0,   5,   5,   5,   5,   0,  -5,
    -10,   0,   5,   5,   5,   5,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20
};

constexpr int KingTable[64] = {
     20,  30,  10,   0,   0,  10,  30,  20,
     20,  20,   0,   0,   0,   0,  20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};

int mirror_square(int square) {
    int file = file_of(square);
    int rank = rank_of(square);

    return (7 - rank) * 8 + file;
}

const int* table_for_piece(PieceType piece_type) {
    switch (piece_type) {
        case PieceType::Pawn:
            return PawnTable;
        case PieceType::Knight:
            return KnightTable;
        case PieceType::Bishop:
            return BishopTable;
        case PieceType::Rook:
            return RookTable;
        case PieceType::Queen:
            return QueenTable;
        case PieceType::King:
            return KingTable;
        case PieceType::None:
            return nullptr;
    }

    return nullptr;
}

int material_balance(const Board& board) {
    int score = 0;

    for (Piece piece : board.squares) {
        int value = piece_value(piece.type);

        if (piece.color == Color::White) {
            score += value;
        } else {
            score -= value;
        }
    }

    return score;
}

int non_king_material(const Board& board) {
    int material = 0;

    for (Piece piece : board.squares) {
        if (piece.type != PieceType::King) {
            material += piece_value(piece.type);
        }
    }

    return material;
}

int king_square_for_color(const Board& board, Color color) {
    for (int square = 0; square < 64; ++square) {
        Piece piece = board.squares[square];

        if (piece.type == PieceType::King && piece.color == color) {
            return square;
        }
    }

    return NoSquare;
}

int distance_between(int first_square, int second_square) {
    int file_distance = file_of(first_square) - file_of(second_square);
    int rank_distance = rank_of(first_square) - rank_of(second_square);

    if (file_distance < 0) {
        file_distance = -file_distance;
    }

    if (rank_distance < 0) {
        rank_distance = -rank_distance;
    }

    return file_distance + rank_distance;
}

int edge_distance(int square) {
    int file = file_of(square);
    int rank = rank_of(square);
    int file_edge_distance = file < 7 - file ? file : 7 - file;
    int rank_edge_distance = rank < 7 - rank ? rank : 7 - rank;

    return file_edge_distance < rank_edge_distance ? file_edge_distance : rank_edge_distance;
}

int endgame_mop_up_bonus(const Board& board) {
    int material = material_balance(board);

    if (material > -500 && material < 500) {
        return 0;
    }

    if (non_king_material(board) > 2200) {
        return 0;
    }

    Color winning_color = material > 0 ? Color::White : Color::Black;
    Color losing_color = opposite(winning_color);
    int winning_king = king_square_for_color(board, winning_color);
    int losing_king = king_square_for_color(board, losing_color);

    if (winning_king == NoSquare || losing_king == NoSquare) {
        return 0;
    }

    int edge_bonus = (3 - edge_distance(losing_king)) * 30;
    int king_distance_bonus = (14 - distance_between(winning_king, losing_king)) * 4;
    int bonus = edge_bonus + king_distance_bonus;

    return winning_color == Color::White ? bonus : -bonus;
}

}

int piece_value(PieceType piece_type) {
    switch (piece_type) {
        case PieceType::Pawn:
            return 100;
        case PieceType::Knight:
            return 320;
        case PieceType::Bishop:
            return 330;
        case PieceType::Rook:
            return 500;
        case PieceType::Queen:
            return 900;
        case PieceType::King:
        case PieceType::None:
            return 0;
    }
    return 0;
}

int piece_square_value(Piece piece, int square) {
    const int* table = table_for_piece(piece.type);

    if (table == nullptr || !is_valid_square(square)) {
        return 0;
    }

    int table_square = piece.color == Color::White ? square : mirror_square(square);

    return table[table_square];
}

int evaluate(const Board& board) {
    int score = 0;

    for (int square = 0; square < 64; ++square) {
        Piece piece = board.squares[square];
        int value = piece_value(piece.type) + piece_square_value(piece, square);

        if (piece.color == Color::White) {
            score += value;
        } else {
            score -= value;
        }
    }

    return score + endgame_mop_up_bonus(board);
}

}
