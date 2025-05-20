#pragma once
#include <cstdint>
#include "const.hpp"

struct Move_History_Stack {
    uint64_t data[MAX_DEPTH*2];
    int top = 0;
    inline void push(uint64_t m)  { data[top++] = m; }
    inline uint64_t pop()         { return data[--top]; }
    inline uint64_t peek()        { return data[top-1]; }
};

struct MoveStack {
    uint64_t data[64];
    int top = 0;
    inline void push(uint64_t m)  { data[top++] = m; }
    inline uint64_t pop()         { return data[--top]; }
    inline uint64_t peek()        { return data[top-1]; }
};

MoveStack move_generation(uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR);
void undo(MoveStack stack, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB);
void move(MoveStack stack, uint64_t from, uint64_t to, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB);
