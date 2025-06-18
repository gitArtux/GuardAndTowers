#pragma once
#include <cstdint>
#include "const.hpp"
#include <vector>
#include <array>

using Moves = std::vector<std::array<uint64_t,2>>;



struct Move_History_Stack {
    uint64_t data[MAX_DEPTH*2];
    int top = 0;
    inline void push(uint64_t m)  { data[top++] = m; }
    inline uint64_t pop()         { return data[--top]; }
    inline uint64_t peek()        { return data[top-1]; }
};

// struct Moves {
//     uint64_t data[64];
//     int top = 0;
//     inline void push(uint64_t m)  { data[top++] = m; }
//     inline uint64_t pop()         { return data[--top]; }
//     inline uint64_t peek()        { return data[top-1]; }
// };

Moves move_generation(uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint64_t guardB, uint64_t guardR);
void undo(Move_History_Stack stack, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB);
void move(Move_History_Stack stack, uint64_t from, uint64_t to, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB);
