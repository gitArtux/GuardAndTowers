#pragma once
#include <array>
#include <vector>
#include <cstdint>

using Move = std::array<uint64_t, 2>;
using Moves = std::vector<Move>; // Define Moves as a vector of arrays of two uint64_t values
struct ScoredMove {
    Move move;  // Replace with your actual move type (e.g., uint16_t, struct, etc.)
    int score;

    bool operator<(const ScoredMove& other) const {
        return score > other.score;
    }
};
using ScoredMoves = std::vector<ScoredMove>;
using ScoredMoveStack = std::vector<ScoredMove>;
using MoveHistory = std::vector<Move>; // Define MoveHistory as a vector of arrays of two uint64_t values
