#pragma once
#include "types.hpp"



Moves move_generation(uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint64_t guardB, uint64_t guardR);
void undo(Move move, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR);
void do_move(Move &move, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR);
