#pragma once

#include <cstdint>
#include <istream>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "chess/board/board.hpp"

namespace chess {

class OpeningBook {
public:
    explicit OpeningBook(std::uint32_t seed = std::random_device{}());

    bool load(const std::string& path);
    void load_default();
    std::optional<Move> probe(const Board& board);

private:
    struct Entry {
        std::string move;
        double weight;
    };

    using Positions = std::unordered_map<std::uint64_t, std::vector<Entry>>;

    bool read(std::istream& input);

    Positions positions_;
    std::mt19937 random_;
};

}
