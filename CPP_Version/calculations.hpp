#pragma once
#include <cstdint>
#include "const.hpp"

struct MoveStack {
    uint64_t data[MAX_DEPTH*2];
    int top = 0;
    inline void push(uint64_t m)  { data[top++] = m; }
    inline uint64_t pop()         { return data[--top]; }
};

void move_generation(uint64_t (&moves)[24][2], uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR);
void undo(MoveStack stack, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB);
void move(MoveStack stack, uint64_t from, uint64_t to, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB);
void extract_move(uint64_t &to, uint64_t &from, uint64_t &startpos, uint64_t &endpos);