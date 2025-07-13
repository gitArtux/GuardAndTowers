#include <iostream>
#include "board.hpp"
#include "calculations.hpp"
#include "const.hpp"
#include "alpha_beta.hpp"


int main() {

    uint64_t figuresB[7];
    uint64_t figuresR[7];
    uint8_t figuresB_2d[49];
    uint8_t figuresR_2d[49];
    uint64_t guardB;
    uint64_t guardR;
    bool isBlueTurn;
    int depth = 5;

    std::string fen_pos = "r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r";
    set_board(fen_pos, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);
    print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    // print_board2d_red(figuresR_2d);
    // print_board2d_blue(figuresB_2d);
    Move move  = alpha_beta(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
    std::cout << "Best Move: " << FEN_Move(move) << std::endl; // Print the best move in FEN format
    // std::cout << "EVAL: " << evaluate(figuresB, figuresR, guardB, guardR, isBlueTurn) << std::endl; // Print the evaluation of the position
    
    return 0;
}
