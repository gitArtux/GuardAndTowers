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

void test_move_generation(std::string FEN_position, std::string FEN_moves_expected, uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int depth) {
    set_board(FEN_position,moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn); // Set the board to a specific position
    
    std::string fen_moves;
    if (isBlueTurn){
        move_generation(moves[0], figuresB, figuresR, guardB, guardR);
        fen_moves = extract_FEN_Moves(moves[0]); // Extract the FEN moves
    } else {
        move_generation(moves[0], figuresR, figuresB, guardR, guardB);
        fen_moves = extract_FEN_Moves(moves[0]); // Extract the FEN moves
    }

    std::vector<std::string_view> tokens_expected = split(FEN_moves_expected);
    std::vector<std::string_view> tokens_generated = split(fen_moves);
    

    if (same_elements_sorted(tokens_expected, tokens_generated)) {
        std::cout << "\033[32mO\033[0m Test succeeded for: '" << FEN_position << "' " << std::endl;
    } else {
        std::cout << "\033[31mX\033[0m Test failed for: '" << FEN_position << "' " << std::endl;
        std::cout << "Expected moves: " << FEN_moves_expected << std::endl;
        std::cout << "Generated moves: " << fen_moves << std::endl;
        print_board(figuresB, figuresR, guardB, guardR); // Print the board
    }
}

// Test cases for move generation: map FEN positions to expected move lists
// FEN strings go from the top of the board to the bottom, and from left to right. As per server
static const std::map<std::string, std::string> TEST_CASES = {
    {"RGBG5/7/7/7/7/7/7 b", "B7-A7-1 B7-B6-1 B7-C7-1"},
    {"3RG3/7/7/7/7/7/3BG3 r", "D7-C7-1 D7-D6-1 D7-E7-1"},
    {"r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r", "A7-A6-1 A7-B7-1 B7-A7-1 B7-B6-1 B7-C7-1 C6-B6-1 C6-C5-1 C6-C7-1 C6-D6-1 D5-C5-1 D5-D4-1 D5-D6-1 D5-E5-1 D7-C7-1 D7-D6-1 D7-E7-1 E6-D6-1 E6-E5-1 E6-E7-1 E6-F6-1 F7-E7-1 F7-F6-1 F7-G7-1 G7-F7-1 G7-G6-1"},
};

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

    // test_move_generation("RGBG5/7/7/7/7/7/7 b", "B7-A7-1 B7-B6-1 B7-C7-1", moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);





    for (const auto& case_pair : TEST_CASES) {
        const auto& fen = case_pair.first;
        const auto& expected = case_pair.second;
        test_move_generation(
            fen, expected,
            moves, figuresB, figuresR, figuresB_2d, figuresR_2d,
            guardB, guardR, isBlueTurn, depth
        );
    }


    return 0;
}



