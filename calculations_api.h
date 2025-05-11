#pragma once
#include <cstdint>

void move_generation(uint64_t (&moves)[24][2], uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR);
bool move(uint64_t from, uint64_t to, uint64_t* __restrict figuresB, uint64_t* __restrict figuresR, uint8_t* __restrict figuresB_2d, uint8_t* figuresR_2d, uint64_t &guardB, uint64_t guardR);
void extract_move(uint64_t &to, uint64_t &from, uint64_t &startpos, uint64_t &endpos);