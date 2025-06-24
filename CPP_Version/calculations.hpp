#pragma once
#include <cstdint>
#include "const.hpp"
#include <vector>
#include <array>

using Move = std::array<uint64_t, 2>;
using Moves = std::vector<Move>;
using MoveStack = std::vector<Moves>;
using MoveHistory = std::vector<Move>; // Define MoveHistory as a vector of arrays of two uint64_t values




Moves move_generation(uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint64_t guardB, uint64_t guardR);
void undo(MoveHistory &stack, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB);
bool do_move(MoveHistory &stack, Move move, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t guardR, uint64_t home);
