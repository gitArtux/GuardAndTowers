#pragma once
#include <cstdint>
#include "const.hpp"
#include <vector>
#include <array>

using Moves = std::vector<std::array<uint64_t,2>>;
using MoveStack = std::vector<Moves>;
using MoveHistory = std::vector<std::array<uint64_t, 2>>; // Define MoveHistory as a vector of arrays of two uint64_t values


// struct Moves {
//     uint64_t data[64];
//     int top = 0;
//     inline void push(uint64_t m)  { data[top++] = m; }
//     inline uint64_t pop()         { return data[--top]; }
//     inline uint64_t peek()        { return data[top-1]; }
// };

Moves move_generation(uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint64_t guardB, uint64_t guardR);
void undo(MoveHistory stack, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB);
bool move(MoveHistory stack, uint64_t from, uint64_t to, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t guardR, uint64_t home);
