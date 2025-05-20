#include <iostream>
#include "board.hpp"

int main() {
    uint64_t moves[MAX_DEPTH][24][2];
    uint64_t figuresB[7];
    uint64_t figuresR[7];
    uint8_t figuresB_2d[49];
    uint8_t figuresR_2d[49];
    uint64_t guardB;
    uint64_t guardR;
    bool isBlueTurn;
    int depth;

    std::string fen_pos = "r3RG5/r16/b16/7/7/7/3BG3 r";
    set_board(fen_pos, moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);
    print_board(figuresB, figuresR, guardB, guardR);
    print_Bitboard(figuresR[0]);
    print_Bitboard(figuresR[1]);
    print_Bitboard(figuresR[2]);
    print_Bitboard(figuresR[3]);
    print_Bitboard(figuresR[4]);
    print_Bitboard(figuresR[5]);
    print_Bitboard(figuresR[6]);

    

    
    return 0;
}