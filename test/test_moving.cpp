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

bool compare_states(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth,
uint64_t (&expected_figuresB)[7], uint64_t (&expected_figuresR)[7], uint8_t (&expected_figuresB_2d)[49], uint8_t (&expected_figuresR_2d)[49], uint64_t &expected_guardB, uint64_t &expected_guardR, bool &expected_isBlueTurn, int &expected_depth){

    // Compare the current state with the expected state
    return (
        // stack == expected_stack &&
        // movehistory == expected_movehistory &&
        std::equal(std::begin(figuresB), std::end(figuresB), std::begin(expected_figuresB)) &&
        std::equal(std::begin(figuresR), std::end(figuresR), std::begin(expected_figuresR)) &&
        std::equal(std::begin(figuresB_2d), std::end(figuresB_2d), std::begin(expected_figuresB_2d)) &&
        std::equal(std::begin(figuresR_2d), std::end(figuresR_2d), std::begin(expected_figuresR_2d)) &&
        guardB == expected_guardB &&
        guardR == expected_guardR 
        // isBlueTurn == expected_isBlueTurn &&
        // depth == expected_depth
    );
}

void test_move_undo_depth1(std::string fen, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth) {
    set_board(fen, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);

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



    if (isBlueTurn){
        Moves moves = move_generation(figuresB, figuresR, figuresB_2d, guardB, guardR);
        std::cout << "Before: " << std::endl; // Print the moves

        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
        // debug(figuresB, figuresR, guardB, guardR, isBlueTurn); // Debug information
        
        do_move(moves[0], figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR);
        std::cout << "After: " << std::endl; // Print the moves
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
        undo(moves[0], figuresB, figuresR, figuresB_2d, figuresR_2d, guardB); // Undo the move
        std::cout << "After undo: " << std::endl; // Print the moves

        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
        // print_board2d_blue(figuresB_2d); // Print the blue figures in 2D
        // debug(figuresB, figuresR, guardB, guardR, isBlueTurn); // Debug information


    } else {
        Moves moves = move_generation(figuresR, figuresB, figuresR_2d, guardR, guardB);
        std::cout << "Before: " << std::endl; // Print the moves
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board

        do_move(moves[0], figuresR, figuresB, figuresR_2d, figuresB_2d, guardR, guardB);
        std::cout << "After: " << std::endl; // Print the moves
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
        
        undo(moves[0], figuresR, figuresB, figuresR_2d, figuresB_2d, guardR); // Undo the move
        std::cout << "After undo: " << std::endl; // Print the moves
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board

    }
    

    bool expected_result = compare_states(
        figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth,
        expected_figuresB, expected_figuresR, expected_figuresB_2d, expected_figuresR_2d, expected_guardB, expected_guardR, expected_isBlueTurn, expected_depth
    );

    if (expected_result) {
        std::cout << "\033[32mO\033[0m Test succeeded for: '" << fen << "' " << std::endl;

        // print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    } else {
        std::cout << "\033[31mX\033[0m Test failed for: '" << fen << "' " << std::endl;
        // std::cout << "Figures B: " << std::endl;
        // print_Bitboard(figuresB[0]);
        // std::cout << "Figures R: " << std::endl;
        // print_Bitboard(figuresR[0]);
        // print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board

    }
}

void test_moves(std::string fen, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth) {
    set_board(fen, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);


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


    if (isBlueTurn){
        Moves moves = move_generation(figuresB, figuresR, figuresB_2d, guardB, guardR);

        for (auto& move : moves) {
            // print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
            do_move(move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR);
            undo(move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB); // Undo the move

        }

    } else {
        Moves moves = move_generation(figuresR, figuresB, figuresR_2d, guardR, guardB);
        for (auto& move : moves) {
            // print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
            do_move(move, figuresR, figuresB, figuresR_2d, figuresB_2d, guardR, guardB);
            undo(move, figuresR, figuresB, figuresR_2d, figuresB_2d, guardR); // Undo the move
        }
        
    }
    

    bool expected_result = compare_states(
        figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth,
        expected_figuresB, expected_figuresR, expected_figuresB_2d, expected_figuresR_2d, expected_guardB, expected_guardR, expected_isBlueTurn, expected_depth
    );


    if (expected_result) {
        std::cout << "\033[32mO\033[0m Test succeeded for: '" << fen << "' " << std::endl;

        // print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    } else {
        std::cout << "\033[31mX\033[0m Test failed for: '" << fen << "' " << std::endl;
        // std::cout << "Figures B: " << std::endl;
        // print_Bitboard(figuresB[0]);
        // std::cout << "Figures R: " << std::endl;
        // print_Bitboard(figuresR[0]);
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board

    }
}

static const std::map<std::string, std::string> TEST_CASES_UNDO = {
    //  random state
    {"3RG3/7/3r13/3b13/7/7/3BG3 b", "D4-D5-1"},
    {"3RG3/3r23/7/3b33/7/7/3BG3 b", "D4-D6-2"},
    {"3RG3/3r33/7/7/3b33/7/3BG3 b", "D3-D6-3"},
    //  only two guards left
    // {"3RG3/7/7/7/7/7/3BG3 r", "D7-C7-1 D7-D6-1 D7-E7-1"},
    // //  initial state
    // {"r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r", "A7-A6-1 A7-B7-1 B7-A7-1 B7-B6-1 B7-C7-1 C6-B6-1 C6-C5-1 C6-C7-1 C6-D6-1 D5-C5-1 D5-D4-1 D5-D6-1 D5-E5-1 D7-C7-1 D7-D6-1 D7-E7-1 E6-D6-1 E6-E5-1 E6-E7-1 E6-F6-1 F7-E7-1 F7-F6-1 F7-G7-1 G7-F7-1 G7-G6-1"},
    // //  can't capture with friendly piece in between
    // {"r3RG5/r16/b16/7/7/7/3BG3 r", "A6-A5-1 A6-A7-1 A6-B6-1 A7-A6-1 B7-B6-1 B7-C7-1"},
    // //  correct n=7 tower moves
    // {"b76/3RG3/7/7/7/7/3BG3 b", "A7-A1-6 A7-A2-5 A7-A3-4 A7-A4-3 A7-A5-2 A7-A6-1 A7-B7-1 A7-C7-2 A7-D7-3 A7-E7-4 A7-F7-5 A7-G7-6 D1-C1-1 D1-D2-1 D1-E1-1"},
    // //  blue can't capture higher red tower
    // {"r43RG2/7/7/b36/7/7/3BG3 b", "A4-A1-3 A4-A2-2 A4-A3-1 A4-A5-1 A4-A6-2 A4-B4-1 A4-C4-2 A4-D4-3 D1-C1-1 D1-D2-1 D1-E1-1"},
    // //  can't capture tower of hight 3 if only one field distance
    // {"r62RG3/b36/7/7/7/7/3BG3 r", "A7-B7-1 A7-C7-2 D7-C7-1 D7-D6-1 D7-E7-1"}
};

void test_undo(std::string fen, std::string move_str, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth) {
    set_board(fen, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);
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

    Move move = get_Fen_move(move_str);

    if(isBlueTurn) {
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
        do_move(move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR);
        print_board(figuresB, figuresR, guardB, guardR, false); // Print the board
        undo(move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB); // Undo the move
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board

    } else {
        do_move(move, figuresR, figuresB, figuresR_2d, figuresB_2d, guardR, guardB);
        undo(move, figuresR, figuresB, figuresR_2d, figuresB_2d, guardR); // Undo the move
    }

     bool expected_result = compare_states(
        figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth,
        expected_figuresB, expected_figuresR, expected_figuresB_2d, expected_figuresR_2d, expected_guardB, expected_guardR, expected_isBlueTurn, expected_depth
    );


    if (expected_result) {
        std::cout << "\033[32mO\033[0m Test succeeded for: '" << fen << "' " << std::endl;

        // print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    } else {
        std::cout << "\033[31mX\033[0m Test failed for: '" << fen << "' " << std::endl;
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

    for (const auto& case_pair : TEST_CASES_UNDO) {
        const auto& fen = case_pair.first;
        const auto& moves_str = case_pair.second;
        // test_moves(fen, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
        test_undo(fen, moves_str, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
    }

    

    // test_move_undo_depth1("3RG3/7/7/3b13/7/7/3BG3 b", figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
    return 0;
}