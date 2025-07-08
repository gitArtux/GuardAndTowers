#pragma once
#include <cstdint>
#include <array>
#include <vector>

using Move = std::array<std::uint64_t, 2>;
using Moves = std::vector<Move>;
struct ScoredMove {
    Move move;  // Replace with your actual move type (e.g., uint16_t, struct, etc.)
    int score;
};
using ScoredMoves = std::vector<ScoredMove>;



