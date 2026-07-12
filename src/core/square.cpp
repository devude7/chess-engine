#include "chess/core/square.hpp"

namespace chess {

bool is_valid_square(int square) {
    return square >= 0 && square < 64;
}

int file_of(int square) {
    return square % 8;
}

int rank_of(int square) {
    return square / 8;
}

}
