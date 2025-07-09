#pragma once
#include "types.hpp"
#include "calculations.hpp"
#include "const.hpp"
#include "board.hpp"
#include <array>
#include <vector>
#include <algorithm>
#include <iostream>


Move alpha_beta(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool turnB, int depth);
ScoredMove beta_search(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int depth, int alpha, int beta);
ScoredMove alpha_search(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int depth, int alpha, int beta);
int evaluate(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR, bool isBlueTurn);

std::string make_move(std::string fen_pos, int depth);