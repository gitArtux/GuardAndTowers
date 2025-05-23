#pragma once
#include <cstdint>
#include <string>
#include "const.h"


void test_board_link(const std::string& str); // Basic print function
void clear_board(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR);
void init_board(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth);
void set_board(std::string fen_pos, uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn);

inline std::string FEN_position(std::uint64_t pos);
std::string extract_FEN_Moves(uint64_t (&moves)[24][2]);
void print_board(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR);
void print_Bitboard(std::uint64_t bitboard);
