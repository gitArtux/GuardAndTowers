#include <iostream>
#include <cstdint>
#include <chrono>
#include "calculations.hpp"
#include "board.hpp"

// Benchmarking --------------------------------------------------------------------------------------------------- 
constexpr int NUM_RUNS = 10000000; // Number of tests to run
void benchmark_generator(uint64_t (&moves)[24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int &depth) {
    volatile uint64_t dummy_0 = 0; // Dummy variable to prevent optimization
    volatile uint64_t dummy_1 = 0; // Dummy variable to prevent optimization

    auto start = std::chrono::steady_clock::now(); // Start the timer
    for (int i = 0; i < NUM_RUNS; ++i) {
        // figuresB[i%7] = 1ULL << (i % 64);

        move_generation(moves, figuresB, figuresR, guardB, guardR);
        dummy_0 ^= moves[i % 16][0];
        dummy_1 ^= moves[i % 16][1];
    }
    auto end = std::chrono::steady_clock::now(); // Stop the timer
    std::cout << "Dummy 0: " << dummy_0 << std::endl; // Print the dummy variable
    std::cout << "Dummy 1: " << dummy_1 << std::endl; // Print the dummy variable
    std::chrono::duration<double> elapsed = end - start; // Calculate the elapsed time
    std::cout << "Time taken for " << NUM_RUNS << " runs: " << elapsed.count() << " seconds" << std::endl; // Print the elapsed time
}

void dummy_benchmark(uint64_t (&moves)[24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int &depth){

    volatile uint64_t dummy_0 = 0; // Dummy variable to prevent optimization
    volatile uint64_t dummy_1 = 0; // Dummy variable to prevent optimization
    volatile uint64_t dummy_2 = 0; // Dummy variable to prevent optimization


    auto start = std::chrono::steady_clock::now(); // Start the timer
    for (int i = 0; i < NUM_RUNS; ++i) {

        dummy_2 ^= figuresB[i%7] = 1ULL << (i % 64);

        // move_generationB(moves[0], figuresB, figuresR, guardB, guardR);
        dummy_0 ^= moves[i%24][0];
        dummy_1 ^= moves[i%24][1];
    }
    auto end = std::chrono::steady_clock::now(); // Stop the timer
    std::cout << "Dummy 0: " << dummy_0 << std::endl; // Print the dummy variable
    std::cout << "Dummy 1: " << dummy_1 << std::endl; // Print the dummy variable
    std::cout << "Dummy 2: " << dummy_2 << std::endl; // Print the dummy variable
    std::chrono::duration<double> elapsed = end - start; // Calculate the elapsed time
    std::cout << "Time taken for " << NUM_RUNS << " runs: " << elapsed.count() << " seconds" << std::endl; // Print the elapsed time
}

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
    // set_board(fen_pos, moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn); // Set the board to a specific position
    init_board(moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth); // Set the board to a specific position
    dummy_benchmark(moves[0], figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, depth);
    benchmark_generator(moves[0], figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, depth);

    
    return 0;
}