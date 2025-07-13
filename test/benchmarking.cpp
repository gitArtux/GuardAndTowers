#include <iostream>
#include <cstdint>
#include <chrono>
#include "calculations.hpp"
#include "board.hpp"
#include "alpha_beta.hpp"

// Benchmarking --------------------------------------------------------------------------------------------------- 
constexpr int NUM_RUNS = 10000000; // Number of tests to run
void benchmark_generator(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int &depth) {
    volatile uint64_t dummy_0 = 0; // Dummy variable to prevent optimization
    for (int i = 0; i < 10000; ++i) {

        Moves moves =  move_generation(figuresB, figuresR, figuresB_2d, guardB, guardR);
        dummy_0 += moves.back()[0] + moves.back()[1]; // Dummy operation to prevent optimization

    }
    auto start = std::chrono::steady_clock::now(); // Start the timer
    for (int i = 0; i < NUM_RUNS; ++i) {

        Moves moves =  move_generation(figuresB, figuresR, figuresB_2d, guardB, guardR);
        dummy_0 += moves.back()[0] + moves.back()[1]; // Dummy operation to prevent optimization

    }
    auto end = std::chrono::steady_clock::now(); // Stop the timer
    std::cout << "Dummy 0: " << dummy_0 << std::endl; // Print the dummy variable
    std::chrono::duration<double> elapsed = end - start; // Calculate the elapsed time
    std::cout << "Time taken for " << NUM_RUNS << " runs: " << elapsed.count() << " seconds" << std::endl; // Print the elapsed time
}

void dummy_benchmark(){
    volatile uint64_t dummy_0 = 0; // Dummy variable to prevent optimization
    for (int i = 0; i < 10000; ++i) {
         dummy_0 += 54; // Dummy operation
    }

    auto start = std::chrono::steady_clock::now(); // Start the timer
    for (int i = 0; i < NUM_RUNS; ++i) {
        dummy_0 += 54; // Dummy operation
    }
    auto end = std::chrono::steady_clock::now(); // Stop the timer
    std::cout << "Dummy 0: " << dummy_0 << std::endl; // Print the dummy variable
    std::chrono::duration<double> elapsed = end - start; // Calculate the elapsed time
    std::cout << "Time taken for " << NUM_RUNS << " runs: " << elapsed.count() << " seconds" << std::endl; // Print the elapsed time
}

void alpha_beta_benchmark() {
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
    // print_board(figuresB, figuresR, guardB, guardR, isBlueTurn); // Print the board
    // print_board2d_red(figuresR_2d);
    // print_board2d_blue(figuresB_2d);
    auto start = std::chrono::steady_clock::now(); // Start the timer

    Move move  = alpha_beta(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth);
    // std::cout << "EVAL: " << evaluate(figuresB, figuresR, guardB, guardR, isBlueTurn) << std::endl; // Print the evaluation of the position
    

    auto end = std::chrono::steady_clock::now(); // Stop the timer after the alpha-beta search
    std::chrono::duration<double> elapsed = end - start; // Calculate the elapsed time
    // std::cout << "Best Move: " << FEN_Move(move) << std::endl; // Print the best move in FEN format
    std::cout << "Best Move: " << FEN_Move(move) << std::endl; // Print the best move in FEN format

    std::cout << "Time taken for:" << elapsed.count() << " seconds" << std::endl; // Print the elapsed time
}
int main() {
    std::cout << "Alpha Beta" << std::endl;
    alpha_beta_benchmark(); // Run the alpha-beta benchmark

    std::cout << "Move generation" << std::endl;

    uint64_t figuresB[7];
    uint64_t figuresR[7];
    uint8_t figuresB_2d[49];
    uint8_t figuresR_2d[49];
    uint64_t guardB;
    uint64_t guardR;
    bool isBlueTurn;
    int depth = 5; // Set the depth for the alpha-beta search

    std::string fen_pos = "r1r11RG1r1r1/2r11r12/7/7/7/2b11b12/b1b11BG1b1b1 r";
    set_board(fen_pos, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn); // dummy_benchmark();
    benchmark_generator(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, depth);
    // dummy_benchmark();
    
    return 0;
}