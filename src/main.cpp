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
    int depth = 7;
    
    std::string fen_pos = "r22RG1r21/2r2r13/7/4b32/b22b13/2b1BG3/7 r";

    set_board(fen_pos, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);
    print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    // print_board2d_red(figuresR_2d);
    // print_board2d_blue(figuresB_2d);
    Move move  = alpha_beta(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
    std::cout << "Best Move: " << FEN_Move(move) << std::endl; // Print the best move in FEN format
    
    return 0;
}
