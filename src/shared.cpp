// import this file as module in Python using pybind11

#include <iostream>
#include <string>
#include "board.hpp"
#include "calculations.hpp"
#include "const.hpp"
#include "alpha_beta.hpp"

std::string compute_best_move(const std::string& fen_pos, int depth) {
    uint64_t figuresB[7];
    uint64_t figuresR[7];
    uint8_t figuresB_2d[49];
    uint8_t figuresR_2d[49];
    uint64_t guardB;
    uint64_t guardR;
    bool isBlueTurn;

    set_board(fen_pos, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);
    // Optionally remove or comment out print_board()
    Move move = alpha_beta(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
    return FEN_Move(move);
}