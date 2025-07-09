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

    std::string fen_pos = "6r2/RG6/1b21b2b1b11/7/1b15/2r11r12/2r1BG3 b";
    set_board(fen_pos, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);
    print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    // print_board2d_red(figuresR_2d);
    // print_board2d_blue(figuresB_2d);
    Move move  = alpha_beta(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
    std::cout << "Best Move: " << FEN_Move(move) << std::endl; // Print the best move in FEN format
    
    return 0;
}
