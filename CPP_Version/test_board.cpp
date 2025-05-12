#include "calculations.h"
#include "board.h"


// Temporary includes for printing and FEN
#include <iostream>
#include <vector>
#include <algorithm>


void test_move_generation(std::string FEN_position, std::string FEN_moves_expected, uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool isBlueTurn, int depth) {
    set_board(FEN_position,moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn); // Set the board to a specific position
    
    std::string fen_moves;
    if (isBlueTurn){
        move_generation(moves[0], figuresB, figuresR, guardB, guardR);
        fen_moves = extract_FEN_Moves(moves[0]); // Extract the FEN moves
    } else {
        move_generation(moves[0], figuresR, figuresB, guardR, guardB);
        fen_moves = extract_FEN_Moves(moves[0]); // Extract the FEN moves
    }

    

    // Split FEN_moves_expected and fen_moves at ", "
    std::vector<std::string> expected_moves;
    std::vector<std::string> generated_moves;

    std::string FEN_moves_expected_copy = FEN_moves_expected; // Create a copy of the expected moves string
    std::string fen_moves_copy = fen_moves; // Create a copy of the generated moves string

    size_t pos = 0;
    while ((pos = FEN_moves_expected.find(", ")) != std::string::npos) {
        expected_moves.push_back(FEN_moves_expected.substr(0, pos));
        FEN_moves_expected.erase(0, pos + 2);
    }
    if (!FEN_moves_expected.empty()) {
        expected_moves.push_back(FEN_moves_expected);
    }

    pos = 0;
    while ((pos = fen_moves.find(", ")) != std::string::npos) {
        generated_moves.push_back(fen_moves.substr(0, pos));
        fen_moves.erase(0, pos + 2);
    }
    if (!fen_moves.empty()) {
        generated_moves.push_back(fen_moves);
    }

    // Check if all expected moves are in generated moves
    bool all_moves_present = true;
    for (const auto& move : expected_moves) {
        if (std::find(generated_moves.begin(), generated_moves.end(), move) == generated_moves.end()) {
            all_moves_present = false;

        }
    }

    // Check if the number of moves matches
    bool move_count_matches = (expected_moves.size() == generated_moves.size());

    if (all_moves_present && move_count_matches) {
        std::cout << "\033[32mO\033[0m Test succeeded for: '" << FEN_position << "' " << std::endl;
    } else {
        std::cout << "\033[31mX\033[0m Test failed for: '" << FEN_position << "' " << std::endl;
        std::cout << "Expected moves: " << FEN_moves_expected_copy << std::endl;
        std::cout << "Generated moves: " << fen_moves_copy << std::endl;
        print_board(figuresB, figuresR, guardB, guardR); // Print the board
    }
}

int main(){
    // Test cases
    uint64_t moves[MAX_DEPTH][24][2];
    uint64_t figuresB[7];
    uint64_t figuresR[7];
    uint8_t figuresB_2d[49];
    uint8_t figuresR_2d[49];
    uint64_t guardB;
    uint64_t guardR;
    bool isBlueTurn;
    int depth;

    test_move_generation("RGBG5/7/7/7/7/7/7 b", "A6-A5-1, A6-B6-1, A6-A7-1", moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);

    return 0;
}



