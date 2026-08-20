#include "chess/search/evaluation.hpp"

#include "chess/core/square.hpp"

namespace chess {

namespace {

constexpr int PassedPawnBonus = 35;
constexpr int DoubledPawnPenalty = 20;
constexpr int IsolatedPawnPenalty = 15;
constexpr int PawnAdvanceBonus = 5;
constexpr int BishopPairBonus = 30;
constexpr int RookSemiOpenFileBonus = 15;
constexpr int RookOpenFileBonus = 30;
constexpr int KingSafetyMaterialThreshold = 2400;
constexpr int KingPawnShieldBonus = 20;
constexpr int KingOpenFilePenalty = 15;
constexpr int KingCenterPenalty = 10;

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

bool has_pawn_on_file(const Board& board, Color color, int file) {
    if (file < 0 || file >= 8) {
        return false;
    }

    for (int rank = 0; rank < 8; ++rank) {
        Piece piece = board.squares[rank * 8 + file];

        if (piece.type == PieceType::Pawn && piece.color == color) {
            return true;
        }
    }

    return false;
}

bool has_any_pawn_on_file(const Board& board, int file) {
    if (file < 0 || file >= 8) {
        return false;
    }

    for (int rank = 0; rank < 8; ++rank) {
        Piece piece = board.squares[rank * 8 + file];

        if (piece.type == PieceType::Pawn) {
            return true;
        }
    }

    return false;
}

int pawn_count_on_file(const Board& board, Color color, int file) {
    int count = 0;

    if (file < 0 || file >= 8) {
        return count;
    }

    for (int rank = 0; rank < 8; ++rank) {
        Piece piece = board.squares[rank * 8 + file];

        if (piece.type == PieceType::Pawn && piece.color == color) {
            ++count;
        }
    }

    return count;
}

bool is_passed_pawn(const Board& board, int square, Color color) {
    int file = file_of(square);
    int rank = rank_of(square);
    Color enemy_color = opposite(color);
    int rank_step = color == Color::White ? 1 : -1;

    for (int checked_file = file - 1; checked_file <= file + 1; ++checked_file) {
        if (checked_file < 0 || checked_file >= 8) {
            continue;
        }

        for (int checked_rank = rank + rank_step; checked_rank >= 0 && checked_rank < 8; checked_rank += rank_step) {
            Piece piece = board.squares[checked_rank * 8 + checked_file];

            if (piece.type == PieceType::Pawn && piece.color == enemy_color) {
                return false;
            }
        }
    }

    return true;
}

int pawn_advance_for_color(int square, Color color) {
    int rank = rank_of(square);

    return color == Color::White ? rank : 7 - rank;
}

int pawn_structure_score_for_pawn(const Board& board, int square, Color color) {
    int file = file_of(square);
    int score = pawn_advance_for_color(square, color) * PawnAdvanceBonus;

    if (is_passed_pawn(board, square, color)) {
        score += PassedPawnBonus + pawn_advance_for_color(square, color) * PawnAdvanceBonus;
    }

    if (pawn_count_on_file(board, color, file) > 1) {
        score -= DoubledPawnPenalty;
    }

    if (!has_pawn_on_file(board, color, file - 1) && !has_pawn_on_file(board, color, file + 1)) {
        score -= IsolatedPawnPenalty;
    }

    return color == Color::White ? score : -score;
}

int pawn_structure_score(const Board& board) {
    int score = 0;

    for (int square = 0; square < 64; ++square) {
        Piece piece = board.squares[square];

        if (piece.type == PieceType::Pawn) {
            score += pawn_structure_score_for_pawn(board, square, piece.color);
        }
    }

    return score;
}

int bishop_pair_score(const Board& board) {
    int white_bishops = 0;
    int black_bishops = 0;

    for (Piece piece : board.squares) {
        if (piece.type != PieceType::Bishop) {
            continue;
        }

        if (piece.color == Color::White) {
            ++white_bishops;
        } else {
            ++black_bishops;
        }
    }

    int score = 0;

    if (white_bishops >= 2) {
        score += BishopPairBonus;
    }

    if (black_bishops >= 2) {
        score -= BishopPairBonus;
    }

    return score;
}

int rook_file_score(const Board& board) {
    int score = 0;

    for (int square = 0; square < 64; ++square) {
        Piece piece = board.squares[square];

        if (piece.type != PieceType::Rook) {
            continue;
        }

        int file = file_of(square);
        bool own_pawn_on_file = has_pawn_on_file(board, piece.color, file);

        if (own_pawn_on_file) {
            continue;
        }

        int bonus = has_any_pawn_on_file(board, file) ? RookSemiOpenFileBonus : RookOpenFileBonus;

        if (piece.color == Color::White) {
            score += bonus;
        } else {
            score -= bonus;
        }
    }

    return score;
}

int king_pawn_shield_score(const Board& board, Color color, int king_square) {
    int score = 0;
    int king_file = file_of(king_square);
    int king_rank = rank_of(king_square);
    int shield_rank = color == Color::White ? king_rank + 1 : king_rank - 1;

    if (shield_rank < 0 || shield_rank >= 8) {
        return score;
    }

    for (int file = king_file - 1; file <= king_file + 1; ++file) {
        if (file < 0 || file >= 8) {
            continue;
        }

        Piece piece = board.squares[shield_rank * 8 + file];

        if (piece.type == PieceType::Pawn && piece.color == color) {
            score += KingPawnShieldBonus;
        } else if (!has_pawn_on_file(board, color, file)) {
            score -= KingOpenFilePenalty;
        }
    }

    return score;
}

int king_center_exposure_penalty(int king_square) {
    int file = file_of(king_square);
    int rank = rank_of(king_square);

    if (file >= 2 && file <= 5 && rank >= 2 && rank <= 5) {
        return KingCenterPenalty;
    }

    return 0;
}

int king_safety_score_for_color(const Board& board, Color color) {
    int king_square = king_square_for_color(board, color);

    if (king_square == NoSquare) {
        return 0;
    }

    int score = king_pawn_shield_score(board, color, king_square);
    score -= king_center_exposure_penalty(king_square);

    return color == Color::White ? score : -score;
}

int king_safety_score(const Board& board) {
    if (non_king_material(board) <= KingSafetyMaterialThreshold) {
        return 0;
    }

    return king_safety_score_for_color(board, Color::White)
        + king_safety_score_for_color(board, Color::Black);
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

    return score
        + pawn_structure_score(board)
        + bishop_pair_score(board)
        + rook_file_score(board)
        + king_safety_score(board)
        + endgame_mop_up_bonus(board);
}

}
