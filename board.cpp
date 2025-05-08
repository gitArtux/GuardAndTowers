#include <cstdint>

#include <cstring> // For memset

// Temporary includes for printing and FEN
#include <iostream>
#include <bitset>


// Include for benchmarking
#include <chrono>


//------------------------------------------


/*
@Ben check out the readme



*/


/*
@Arthur / Both
TODO: think about seperating guard moves from figure moves
TODO: handle stalemate
TODO: guard move generator
TEST
*/

static constexpr int MAX_DEPTH = 8; // Max depth for alpha-beta pruning
static constexpr int MAX_MOVES = 32;


static constexpr int MOVE_DIMENSION = 4;
static constexpr int MOVE_SIZE = 6; 
static constexpr uint64_t HOMESQUARE_B = 1ULL << 4;
static constexpr uint64_t HOMESQUARE_R = 1ULL << 45;

// SHIFTS -------------------------------------------------------------------------------------------------------------------
// TODO: shifts for move calculation

//-------------------------------------------------------------------------------------------------------------------------

// MASKS -------------------------------------------------------------------------------------------------------------------
// TODO: set up the masks for the move calculation
// TODO: reorganize for the different dimensions for early stoppings
static constexpr uint64_t MASK_1RIGHT = 0b0000000000000001111110111111011111101111110111111011111101111110;
static constexpr uint64_t MASK_1LEFT  = 0b0000000000000000111111011111101111110111111011111101111110111111;
static constexpr uint64_t MASK_1UP    = 0b0000000000000000000000111111111111111111111111111111111111111111;
static constexpr uint64_t MASK_1DOWN  = 0b0000000000000001111111111111111111111111111111111111111110000000;
static constexpr uint64_t MASK_2RIGHT = 0b0000000000000001111100111110011111001111100111110011111001111100;
static constexpr uint64_t MASK_2LEFT  = 0b0000000000000000011111001111100111110011111001111100111110011111;
static constexpr uint64_t MASK_2UP    = 0b0000000000000000000000000000011111111111111111111111111111111111;
static constexpr uint64_t MASK_2DOWN  = 0b0000000000000001111111111111111111111111111111111100000000000000;
static constexpr uint64_t MASK_3RIGHT = 0b0000000000000001111000111100011110001111000111100011110001111000;
static constexpr uint64_t MASK_3LEFT  = 0b0000000000000000001111000111100011110001111000111100011110001111;
static constexpr uint64_t MASK_3UP    = 0b0000000000000000000000000000000000000111111111111111111111111111;
static constexpr uint64_t MASK_3DOWN  = 0b0000000000000001111111111111111111111111111000000000000000000000;
static constexpr uint64_t MASK_4RIGHT = 0b0000000000000001110000111000011100001110000111000011100001110000;
static constexpr uint64_t MASK_4LEFT  = 0b0000000000000000000111000011100001110000111000011100001110000111;
static constexpr uint64_t MASK_4UP    = 0b0000000000000000000000000000000000000000000111111111111111111111;
static constexpr uint64_t MASK_4DOWN  = 0b0000000000000001111111111111111111110000000000000000000000000000;
static constexpr uint64_t MASK_5RIGHT = 0b0000000000000001100000110000011000001100000110000011000001100000;
static constexpr uint64_t MASK_5LEFT  = 0b0000000000000000000011000001100000110000011000001100000110000011;
static constexpr uint64_t MASK_5UP    = 0b0000000000000000000000000000000000000000000000000011111111111111;
static constexpr uint64_t MASK_5DOWN  = 0b0000000000000001111111111111100000000000000000000000000000000000;
static constexpr uint64_t MASK_6RIGHT = 0b0000000000000001000000100000010000001000000100000010000001000000;
static constexpr uint64_t MASK_6LEFT  = 0b0000000000000000000001000000100000010000001000000100000010000001;
static constexpr uint64_t MASK_6UP    = 0b0000000000000000000000000000000000000000000000000000000001111111;
static constexpr uint64_t MASK_6DOWN  = 0b0000000000000001111111000000000000000000000000000000000000000000;


// MOVE MASKS --------------------------------------------------------------------------------------------------

static constexpr std::uint64_t MASK_LEFT_MOVES[2][MOVE_SIZE] = {
    {MASK_1LEFT, MASK_2LEFT, MASK_3LEFT, MASK_4LEFT, MASK_5LEFT, MASK_6LEFT},
    {MASK_1UP, MASK_2UP, MASK_3UP, MASK_4UP, MASK_5UP, MASK_6UP}
};

static constexpr std::uint64_t MASK_RIGHT_MOVES[2][MOVE_SIZE] = {
    {MASK_1RIGHT, MASK_2RIGHT, MASK_3RIGHT, MASK_4RIGHT, MASK_5RIGHT, MASK_6RIGHT},
    {MASK_1DOWN, MASK_2DOWN, MASK_3DOWN, MASK_4DOWN, MASK_5DOWN, MASK_6DOWN}
};

// SHIFT APPLICATION MASKS --------------------------------------------------------------------------------------------------

static constexpr int SHIFT_1HORIZONTAL = 1;  // 1 horizontal
static constexpr int SHIFT_2HORIZONTAL = 2;  // 2 horizontal
static constexpr int SHIFT_3HORIZONTAL = 3;  // 3 horizontal
static constexpr int SHIFT_4HORIZONTAL = 4;  // 4 horizontal
static constexpr int SHIFT_5HORIZONTAL = 5;  // 5 horizontal
static constexpr int SHIFT_6HORIZONTAL = 6;  // 6 horizontal

static constexpr int SHIFT_1VERTICAL   = 7;  // 1 vertical
static constexpr int SHIFT_2VERTICAL   = 14; // 2 vertical
static constexpr int SHIFT_3VERTICAL   = 21; // 3 vertical
static constexpr int SHIFT_4VERTICAL   = 28; // 4 vertical
static constexpr int SHIFT_5VERTICAL   = 35; // 5 vertical
static constexpr int SHIFT_6VERTICAL   = 42; // 6 vertical


static constexpr int SHIFTS[MOVE_DIMENSION-2][MOVE_SIZE] = {
    {SHIFT_1HORIZONTAL, SHIFT_2HORIZONTAL, SHIFT_3HORIZONTAL, SHIFT_4HORIZONTAL, SHIFT_5HORIZONTAL, SHIFT_6HORIZONTAL},
    {SHIFT_1VERTICAL, SHIFT_2VERTICAL, SHIFT_3VERTICAL, SHIFT_4VERTICAL, SHIFT_5VERTICAL, SHIFT_6VERTICAL} 
};

//------------------------------------------------------------------------------------------------------------

// Figure types/height
static constexpr uint8_t TYPE_INDEX = 61;
static constexpr std::uint64_t MASK_1 = 1ULL << TYPE_INDEX; 
static constexpr std::uint64_t MASK_2 = 2ULL << TYPE_INDEX; 
static constexpr std::uint64_t MASK_3 = 3ULL << TYPE_INDEX; 
static constexpr std::uint64_t MASK_4 = 4ULL << TYPE_INDEX; 
static constexpr std::uint64_t MASK_5 = 5ULL << TYPE_INDEX; 
static constexpr std::uint64_t MASK_6 = 6ULL << TYPE_INDEX; 
static constexpr std::uint64_t MASK_7 = 7ULL << TYPE_INDEX; 

static constexpr std::uint64_t MASK_STACKHEIGHT = 7ULL << TYPE_INDEX; // TODO: how to handle guard?
static constexpr std::uint64_t MASK_TYPE[7]={MASK_1, MASK_2, MASK_3, MASK_4, MASK_5, MASK_6, MASK_7};

// Board Mask
static constexpr std::uint64_t MASK_FIGURE_TYPE = 7ULL << TYPE_INDEX; 
static constexpr std::uint64_t MASK_BOARD = (1ULL << 49) - 1;
//------------------------------------------------------------------------------------------------------------

static void init_board(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t &guardB, uint64_t &guardR, int &depth) {
    figuresB[0] = 0b0000000000000000000000000000000000000000000000100000101001100011;
    figuresR[0] = 0b0000000000000001100011001010000010000000000000000000000000000000;

    for (int i = 1; i < 7; ++i) {
        figuresB[i] = 0;
        figuresR[i] = 0;
    }

    guardB = 0b0000000000000000000000000000000000000000000000000000000000001000;
    guardR = 0b0000000000000000001000000000000000000000000000000000000000000000;


    std::memset(moves, 0, sizeof(moves)); // Clear the moves array

    depth = 0; // Initialize depth for alpha-beta pruning
}
 
static void move_generationB_1(uint64_t (&moves)[24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR) {
    int adress_count = 0; // Reset adress count for the new generation
    //uint64_t enemy_moves = 0;
    
    // Iterate over 6 possible move sizes
    #pragma GCC unroll 6
    for (int s = 0; s < 6; ++s) {   
        // load into registers for faster access
        uint64_t figsB=figuresB[s];
        uint64_t figsR=figuresR[s];
        uint64_t clear_mask_B=figuresR[s+1] | guardB;
        uint64_t clear_mask_R=figuresB[s+1] | guardR;

        uint64_t mask_left_moves_0 = MASK_LEFT_MOVES[0][s];
        uint64_t mask_left_moves_1 = MASK_LEFT_MOVES[1][s];
        uint64_t mask_right_moves_0 = MASK_RIGHT_MOVES[0][s];
        uint64_t mask_right_moves_1 = MASK_RIGHT_MOVES[1][s];

        uint64_t shift_0 = SHIFTS[0][s];
        uint64_t shift_1 = SHIFTS[1][s];

        uint64_t mask_type = MASK_TYPE[s]; 
             
        // How to make use of ILP
        uint64_t endpos_1 = (figsB &  mask_left_moves_0) << shift_0; // LEFT SHIFTS
        uint64_t endpos_2 = (figsB & mask_left_moves_1) << shift_1; // LEFT SHIFTS
        uint64_t endpos_3 = (figsB & mask_right_moves_0) >> shift_0; // RIGHT SHIFTS
        uint64_t endpos_4 = (figsB & mask_right_moves_1) >> shift_1; // RIGHT SHIFTS

        endpos_1 &= ~(endpos_1 & clear_mask_B); // clear bits where enemy stack is higher or own guard stands
        endpos_2 &= ~(endpos_2 & clear_mask_B); // clear bits where enemy stack is higher or own guard stands
        endpos_3 &= ~(endpos_3 & clear_mask_B); // clear bits where enemy stack is higher or own guard stands
        endpos_4 &= ~(endpos_4 & clear_mask_B); // clear bits where enemy stack is higher or own guard stands

        uint64_t startpos_1 = endpos_1 >> shift_0; // calculate the startpositions by shifting the endpos backwards
        uint64_t startpos_2 = endpos_2 >> shift_1; // calculate the startpositions by shifting the endpos backwards 
        uint64_t startpos_3 = endpos_3 << shift_0; // calculate the startpositions by shifting the endpos backwards
        uint64_t startpos_4 = endpos_4 << shift_1; // calculate the startpositions by shifting the endpos backwards

        moves[adress_count][0] = startpos_1; // store the moves of the dimension in the moves array; notice if no mossives were possible startpos is 0
        moves[adress_count++][1] = endpos_1 | mask_type; // set the stack height of the moving part of the figure

        moves[adress_count][0] = startpos_2; // store the moves of the dimension in the moves array; notice if no mossives were possible startpos is 0
        moves[adress_count++][1] = endpos_2 | mask_type; // set the stack height of the moving part of the figure

        moves[adress_count][0] = startpos_3; // store the moves of the dimension in the moves array; notice if no mossives were possible startpos is 0
        moves[adress_count++][1] = endpos_3 | mask_type; // set the stack height of the moving part of the figure

        moves[adress_count][0] = startpos_4; // store the moves of the dimension in the moves array; notice if no mossives were possible startpos is 0
        moves[adress_count++][1] = endpos_4 | mask_type; // set the stack height of the moving part of the figure

        // endpos_1 = (figsR & MASK_LEFT_MOVES[0][s]) << SHIFTS[0][s]; // enemy moves for the evaluation function
        // endpos_2 = (figsR & MASK_LEFT_MOVES[1][s]) << SHIFTS[1][s]; // enemy moves for the evaluation function
        // endpos_3 = (figsR & MASK_RIGHT_MOVES[0][s]) >> SHIFTS[0][s]; // enemy moves for the evaluation function
        // endpos_4 = (figsR & MASK_RIGHT_MOVES[1][s]) >> SHIFTS[1][s]; // enemy moves for the evaluation function

        // enemy_moves |= endpos_1 & ~(endpos_1 & clear_mask_R); // enemy moves for the evaluation function
        // enemy_moves |= endpos_2 & ~(endpos_2 & clear_mask_R); // enemy moves for the evaluation function
        // enemy_moves |= endpos_3 & ~(endpos_3 & clear_mask_R); // enemy moves for the evaluation function
        // enemy_moves |= endpos_4 & ~(endpos_4 & clear_mask_R); // enemy moves for the evaluation function
    }

    uint64_t shift_0 = SHIFTS[0][0];
    uint64_t shift_1 = SHIFTS[1][0];
    
    moves[0][0] |= guardB; // set the last move to 0 to indicate the end of the moves
    moves[0][1] |= guardB << shift_0; // set the last move to 0 to indicate the end of the moves
    moves[1][0] |= guardB; // set the last move to 0 to indicate the end of the moves
    moves[1][1] |= guardB << shift_1; // set the last move to 0 to indicate the end of the moves
    moves[2][0] |= guardB; // set the last move to 0 to indicate the end of the moves
    moves[2][1] |= guardB >> shift_0; // set the last move to 0 to indicate the end of the moves
    moves[3][0] |= guardB; // set the last move to 0 to indicate the end of the moves
    moves[3][1] |= guardB >> shift_1; // set the last move to 0 to indicate the end of the moves
}       

inline static bool moveB(uint64_t from, uint64_t to, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t &guardB, uint64_t guardR) {
    if (__builtin_expect((!(from & guardB) && !(to & figuresB[0])),1)) {
        uint64_t leaving_height = (to & MASK_STACKHEIGHT) >> TYPE_INDEX;
        uint8_t origin_height = 0;
        uint8_t target_height = 0;
        bool not_found_1 = true;
        bool not_found_2 = true;

        for (int i = 0; i < 7; ++i) {
            uint64_t figsB = figuresB[i];
            
            origin_height ^= (origin_height ^ i) & -not_found_1; // x ^= (x ^ newval) & -cond;
            target_height ^= (target_height ^ i) & -not_found_2; // x ^= (x ^ newval) & -cond;
            
            not_found_1 = from & figsB == 0; 
            not_found_2 = to & figsB == 0; 

            figuresR[i] &= ~(to & -(to & figuresR[i] == 0));
        }

        for (int l = 1; l <= leaving_height; ++l) {
            // Remove the positions from the origin position
            figuresB[origin_height - l] ^= from; 

            // Add the positions to the destination position	
            figuresB[target_height + l] |= to;
        }
    }
    // Guard Move
    guardB ^= (guardB ^ to) & -(from & guardB == 0); // x ^= (x ^ newval) & -cond; 

    // check winconditions --> true if won
    return (from & guardB && to & HOMESQUARE_R) || (to & guardR );
}

// inline static bool moveB() {
//     // Check Wincondition Opponent guard hit
//     if (to & guardR) return true;

//     // Move guard
//     if (from & guardB) {
//         // Check Wincondition Guard on opponent home square
//         if (to & HOMESQUARE_R) return true; 
//         guardB = to; // Move the guard to the new position (guard has height 1)
//     }
//     else {
//         leaving_height = ((to & MASK_STACKHEIGHT) >> TYPE_INDEX); 

//         // Remove the positions from the origin position
//         for (int i = 0; i < 7; ++i) {
//             if (!(from & figuresB[i])) {
//                 for (int l = 1; l <= leaving_height; ++l) {
//                     figuresB[i-l] &= ~from; 
//                 }
//                 break;
//             }
//         }

//         // Add the positions to the destination position
//         to = to & MASK_BOARD; // Remove the stack height from the to position
//         for (int i = 0; i < 7; ++i) {
//             if (!(to & figuresB[i])) {
//                 for (int l = 0; l < leaving_height; ++l) {
//                     figuresB[i+l] |= to; 
//                 }
//                 break;
//             }
//         }
//     }
//     // Remove enemy figure 
//     if (to & figuresR[0]) {
//         for (int i = 0; i < 7; ++i) {
//             if (!(figuresR[i] & to)) break;
//             figuresR[i] &= ~to;
//         }
//     }
//     return false;
// }



// TODO in caller function: while (startpos)
inline static void extract_move(uint64_t &to, uint64_t &from, uint64_t &startpos, uint64_t &endpos) {
    from = 1ULL << __builtin_ctzll(startpos) ; 
    to = 1ULL << __builtin_ctzll(endpos) | MASK_STACKHEIGHT & endpos; // Add stack height of moving part of the figure  
    startpos &= startpos - 1; // Clear the lowest set bit
    endpos &= endpos - 1; // Clear the lowest set bit
}

inline static std::string FEN_position(std::uint64_t pos) {
    uint8_t x = 0;
    uint8_t y = 0;
    int index = __builtin_ctzll(pos); // Get the position of the lowest set bit
    x =  7 - (index % 7); // Column
    y =  6 - (index / 7); // Row  
    char letter = 'G' - y;
    return letter + std::to_string(x); // Convert to string
}

static void print_FEN_Moves(uint64_t (&moves)[24][2]) {

    for(int i = 0; i < MOVE_DIMENSION*MOVE_SIZE; ++i) {
        if (!moves[i][0]) break; // Stop if no more moves are available

        uint64_t startpos = moves[i][0]; // Get the start position
        uint64_t endpos = moves[i][1]; // Get the end position
        uint64_t to;
        uint64_t from;

        uint64_t leaving_height = ((endpos & MASK_STACKHEIGHT) >> TYPE_INDEX); // Get the stack height of the moving part of the figure

        while(startpos) {
            extract_move(to, from, startpos, endpos);
            
            std::cout <<  FEN_position(from) + '-' + FEN_position(to) + '-' + std::to_string(leaving_height) << ", "; // Print the move
        }
    }
    std::cout << std::endl; // Print a new line after all moves

}
  
// Pinting and String Operations -----------------------------------------------------------------------------------
static void print_board(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR) {
    // Empty==0, Blue>0, Red<0
    std::string board[7][7];
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 7; ++j) {
            board[i][j] = "\033[4m| \033[0m";
        }
    }
    std::string type = " "; 
    uint8_t x = 0;
    uint8_t y = 0;
    int pos;

    // Fill the board 
    for (int l = 0; l < 2; ++l){
        // FIGURES
        std::uint64_t* figures = (l == 0) ? figuresB : figuresR;
        for(int h = 0; h < 7; ++h) {
            if (figures[h]){
                uint64_t temp = figures[h];
                while(temp) {
                    pos = __builtin_ctzll(temp);
                    temp &= temp - 1; 
                    x =  6 - (pos % 7); // Column
                    y =  6 - (pos / 7); // Row  
                    type = std::to_string(h + 1); // Convert to string
                    if (l == 0) {
                        board[y][x] = "\033[4m|\033[34m" + type + "\033[0m"; // Blue   
                    } else {
                        board[y][x] = "\033[4m|\033[31m" + type + "\033[0m"; // Red 
                    }
                }
            }
        }
        // GUARD
        std::uint64_t guard = (l == 0) ? guardB : guardR; // Pointer to the current player's figures
        pos = __builtin_ctzll(guard);
        x =  6 - (pos % 7); // Column
        y =  6 - (pos / 7); // Row  
        if (l == 0) {
            board[y][x] = "\033[4m|\033[34mG\033[0m"; // Blue   
        } else {
            board[y][x] = "\033[4m|\033[31mG\033[0m"; // Red 
        }
    }
    // BLUE: \033[34m \033[0m
    // RED: \033[31m \033[0m
    // underline: \033[4m \033[0m

    // Print the board
    std::cout << "   \033[4mBlues Turn     \033[0m" << "\n";
    for (int i = 0; i < 7; ++i) {
        char letter = 'G' - i; // Convert index to letter
        std::cout << letter << "  "; 
        for (int j = 0; j < 7; ++j) {
            std::cout << board[i][j]; 
        }
        std::cout << "\033[4m|\033[0m" << "\n";
    }
    std::cout << "\n" << "    1 2 3 4 5 6 7 " << std::endl; // Column numbers
}
   
static void print_Bitboard(std::uint64_t bitboard) {
    std::cout << "Bitboard: " << std::bitset<64>(bitboard) << std::endl; // Print the bitboard in binary format
}

void temp() {
    uint64_t figuresB[7];
    uint64_t figuresR[7];

    uint64_t guardB;
    uint64_t guardR;

    uint64_t moves[MAX_DEPTH][24][2];

    int depth; // Initialize depth for alpha-beta pruning

    init_board(moves, figuresB, figuresR, guardB, guardR, depth);
    print_board(figuresB, figuresR, guardB, guardR);

    move_generationB_1(moves[0], figuresB, figuresR, guardB, guardR);
    print_FEN_Moves(moves[0]);

}

constexpr int NUM_RUNS = 10000; // Number of tests to run
void benchmark_generator() {


    uint64_t figuresB[7];
    uint64_t figuresR[7];

    uint64_t guardB;
    uint64_t guardR;

    uint64_t moves[MAX_DEPTH][24][2];
    int depth; 

    init_board(moves, figuresB, figuresR, guardB, guardR, depth);

    volatile uint64_t dummy_0 = 0; // Dummy variable to prevent optimization
    volatile uint64_t dummy_1 = 0; // Dummy variable to prevent optimization


    auto start = std::chrono::high_resolution_clock::now(); // Start the timer
    for (int i = 0; i < NUM_RUNS; ++i) {
        move_generationB_1(moves[0], figuresB, figuresR, guardB, guardR);
        dummy_0 ^= moves[0][i % 16][0]; // Dummy operation to prevent optimization
        dummy_1 ^= moves[0][i % 16][1]; // Dummy operation to prevent optimization
    }
    auto end = std::chrono::high_resolution_clock::now(); // Stop the timer

    
    std::chrono::duration<double> elapsed = end - start; // Calculate the elapsed time

    volatile auto dif = elapsed.count();

    std::cout << "Dummy 0: " << dummy_0 << std::endl; // Print the dummy variable
    std::cout << "Dummy 1: " << dummy_1 << std::endl; // Print the dummy variable
    std::cout << "Time taken for " << NUM_RUNS << " runs: " << dif << " seconds" << std::endl; // Print the elapsed time


}



int main() {  
    benchmark_generator();
    std::cout << "Enter to exit" << std::endl;
    std::cin.get();
    return 0;
}
