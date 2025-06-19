#pragma once
#include <cstdint>
#include <string>
#include "const.hpp"
#include <vector>
#include <array> 

using Moves = std::vector<std::array<uint64_t, 2>>; // Define Moves as a vector of arrays of two uint64_t values


void test_board_link(const std::string& str); // Basic print function
void clear_board(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR);
void init_board(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth);
void set_board(std::string fen_pos, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn);

inline std::string FEN_position(std::uint64_t pos);
std::string extract_FEN_Moves(Moves moves);
void print_board(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR, bool isBlueTurn);
void print_Bitboard(std::uint64_t bitboard);
void print_board2d_red(uint8_t (&figuresR_2d)[49]);
void print_board2d_blue(uint8_t (&figuresB_2d)[49]);

