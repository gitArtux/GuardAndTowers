#include "calculations.hpp"
#include "const.hpp"
#include "board.hpp"
#include <iostream>
#include <map>
#include <string>


static const std::map<std::string, std::string> TEST_CASES = {
    //  random state
    {"3RG3/7/7/3b13/7/7/3BG3 b", "D1-C1-1 D1-D2-1 D1-E1-1 D4-C4-1 D4-D3-1 D4-D5-1 D4-E4-1"},
    //  only two guards left
    {"3RG3/7/7/7/7/7/3BG3 r", "D7-C7-1 D7-D6-1 D7-E7-1"},
    //  initial state
    {"r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r", "A7-A6-1 A7-B7-1 B7-A7-1 B7-B6-1 B7-C7-1 C6-B6-1 C6-C5-1 C6-C7-1 C6-D6-1 D5-C5-1 D5-D4-1 D5-D6-1 D5-E5-1 D7-C7-1 D7-D6-1 D7-E7-1 E6-D6-1 E6-E5-1 E6-E7-1 E6-F6-1 F7-E7-1 F7-F6-1 F7-G7-1 G7-F7-1 G7-G6-1"},
    //  can't capture with friendly piece in between
    {"r3RG5/r16/b16/7/7/7/3BG3 r", "A6-A5-1 A6-A7-1 A6-B6-1 A7-A6-1 B7-B6-1 B7-C7-1"},
    //  correct n=7 tower moves
    {"b76/3RG3/7/7/7/7/3BG3 b", "A7-A1-6 A7-A2-5 A7-A3-4 A7-A4-3 A7-A5-2 A7-A6-1 A7-B7-1 A7-C7-2 A7-D7-3 A7-E7-4 A7-F7-5 A7-G7-6 D1-C1-1 D1-D2-1 D1-E1-1"},
    //  blue can't capture higher red tower
    {"r43RG2/7/7/b36/7/7/3BG3 b", "A4-A1-3 A4-A2-2 A4-A3-1 A4-A5-1 A4-A6-2 A4-B4-1 A4-C4-2 A4-D4-3 D1-C1-1 D1-D2-1 D1-E1-1"},
    //  can't capture tower of hight 3 if only one field distance
    {"r62RG3/b36/7/7/7/7/3BG3 r", "A7-B7-1 A7-C7-2 D7-C7-1 D7-D6-1 D7-E7-1"}
};

bool compare_states(MoveStack stack, MoveHistory movehistory, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth,
MoveStack expected_stack, MoveHistory expected_movehistory, uint64_t (&expected_figuresB)[7], uint64_t (&expected_figuresR)[7], uint8_t (&expected_figuresB_2d)[49], uint8_t (&expected_figuresR_2d)[49], uint64_t &expected_guardB, uint64_t &expected_guardR, bool &expected_isBlueTurn, int &expected_depth){

    // Compare the current state with the expected state
    return (
        stack == expected_stack &&
        movehistory == expected_movehistory &&
        figuresB == expected_figuresB &&
        figuresR == expected_figuresR &&
        figuresB_2d == expected_figuresB_2d &&
        figuresR_2d == expected_figuresR_2d &&
        guardB == expected_guardB &&
        guardR == expected_guardR &&
        isBlueTurn == expected_isBlueTurn &&
        depth == expected_depth
    );
}

void test_move_undo_depth1(std::string fen, MoveStack stack, MoveHistory movehistory, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth) {
    set_board(fen, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);
    MoveHistory expected_movehistory;
    MoveStack expected_stack;

    uint64_t expected_figuresB[7];
    uint64_t expected_figuresR[7];
    uint8_t expected_figuresB_2d[49];
    uint8_t expected_figuresR_2d[49];
    std::copy(std::begin(figuresB), std::end(figuresB), std::begin(expected_figuresB));
    std::copy(std::begin(figuresR), std::end(figuresR), std::begin(expected_figuresR));
    std::copy(std::begin(figuresB_2d), std::end(figuresB_2d), std::begin(expected_figuresB_2d));
    std::copy(std::begin(figuresR_2d), std::end(figuresR_2d), std::begin(expected_figuresR_2d));

    uint64_t expected_guardB = guardB;
    uint64_t expected_guardR = guardR;
    bool expected_isBlueTurn = isBlueTurn;
    int expected_depth = depth;

    // Calc Moves
    Moves moves = move_generation(figuresB, figuresR, figuresB_2d, guardB, guardR);
    stack.push_back(moves);
    for (const auto& move : moves) {
        do_move(movehistory, move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, 0);
    }
    


    if () {
        std::cout << "\033[32mO\033[0m Test succeeded for: '" << FEN_position << "' " << std::endl;

        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    } else {
        std::cout << "\033[31mX\033[0m Test failed for: '" << FEN_position << "' " << std::endl;

        // std::cout << "Figures B: " << std::endl;
        // print_Bitboard(figuresB[0]);
        // std::cout << "Figures R: " << std::endl;
        // print_Bitboard(figuresR[0]);
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board

    }
}


int main() {
    uint64_t figuresB[7] = {0};
    uint64_t figuresR[7] = {0};
    uint8_t figuresB_2d[49] = {0};
    uint8_t figuresR_2d[49] = {0};
    uint64_t guardB = 0;
    uint64_t guardR = 0;
    bool isBlueTurn = true;
    int depth = 0;


    


    return 0;
}