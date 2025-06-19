#include "calculations.hpp"
#include "board.hpp"


// Temporary includes for printing and FEN
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

#include <string_view>

std::vector<std::string_view> split(std::string_view sv) {
    std::vector<std::string_view> tokens;
    size_t i = 0, n = sv.size();
    while (i < n) {
        // skip leading spaces
        while (i < n && sv[i] == ' ') ++i;
        if (i >= n) break;
        size_t start = i;
        // find end of token
        while (i < n && sv[i] != ' ') ++i;
        tokens.emplace_back(sv.substr(start, i - start));
    }
    return tokens;
}

bool same_elements_sorted(std::vector<std::string_view> A,
                          std::vector<std::string_view> B) {
    if (A.size() != B.size()) return false;
    std::sort(A.begin(), A.end());
    std::sort(B.begin(), B.end());
    return std::equal(A.begin(), A.end(), B.begin());
}

void test_move_generation(std::string FEN_position, std::string FEN_moves_expected, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int depth) {
    set_board(FEN_position, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn); // Set the board to a specific position
    
    std::string fen_moves;
    if (isBlueTurn){
        Moves moves = move_generation(figuresB, figuresR, figuresB_2d, guardB, guardR);
        fen_moves = extract_FEN_Moves(moves); // Extract the FEN moves
    } else {
        Moves moves = move_generation(figuresR, figuresB, figuresR_2d, guardR, guardB);
        fen_moves = extract_FEN_Moves(moves); // Extract the FEN moves
    }

    std::vector<std::string_view> tokens_expected = split(FEN_moves_expected);
    std::vector<std::string_view> tokens_generated = split(fen_moves);
    

    if (same_elements_sorted(tokens_expected, tokens_generated)) {
        std::cout << "\033[32mO\033[0m Test succeeded for: '" << FEN_position << "' " << std::endl;
        std::cout << "Expected moves: " << FEN_moves_expected << std::endl;
        std::cout << "Generated moves: " << fen_moves << std::endl;
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    } else {
        std::cout << "\033[31mX\033[0m Test failed for: '" << FEN_position << "' " << std::endl;
        std::cout << "Expected moves: " << FEN_moves_expected << std::endl;
        std::cout << "Generated moves: " << fen_moves << std::endl;
        // std::cout << "Figures B: " << std::endl;
        // print_Bitboard(figuresB[0]);
        // std::cout << "Figures R: " << std::endl;
        // print_Bitboard(figuresR[0]);
        print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board

    }
}

// Test cases for move generation: map FEN positions to expected move lists
// FEN strings go from the top of the board to the bottom, and from left to right. As per server
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

int main(){
    // Test cases
    uint64_t figuresB[7];
    uint64_t figuresR[7];
    uint8_t figuresB_2d[49];
    uint8_t figuresR_2d[49];
    uint64_t guardB;
    uint64_t guardR;
    bool isBlueTurn;
    int depth;

    // test_move_generation("r43RG2/7/7/b36/7/7/3BG3 b", "A4-A1-3 A4-A2-2 A4-A3-1 A4-A5-1 A4-A6-2 A4-B4-1 A4-C4-2 A4-D4-3 D1-C1-1 D1-D2-1 D1-E1-1", moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);


    // test_move_generation("r3RG5/r16/b16/7/7/7/3BG3 r", "A6-A5-1 A6-A7-1 A6-B6-1 A7-A6-1 B7-B6-1 B7-C7-1", moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
    // test_move_generation("b3BG5/b16/r16/7/7/7/3RG3 b", "A6-A5-1 A6-A7-1 A6-B6-1 A7-A6-1 B7-B6-1 B7-C7-1", moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);

    for (const auto& case_pair : TEST_CASES) {
        const auto& fen = case_pair.first;
        const auto& expected = case_pair.second;
        test_move_generation(
            fen, expected,
            figuresB, figuresR, figuresB_2d, figuresR_2d,
            guardB, guardR, isBlueTurn, depth
        );
    }
    return 0;
}



