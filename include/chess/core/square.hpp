#pragma once

namespace chess {

constexpr int NoSquare = -1;

bool is_valid_square(int square);
bool is_valid_file_rank(int file, int rank);
int file_of(int square);
int rank_of(int square);

}
