#include "chess/core/square.hpp"

namespace chess {

bool is_valid_square(int square) {
    return square >= 0 && square < 64;
}

bool is_valid_file_rank(int file, int rank) {
    return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

int file_of(int square) {
    return square % 8;
}

int rank_of(int square) {
    return square / 8;
}

}
