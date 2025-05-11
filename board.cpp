#include <cstdint>

#include <cstring> // For memset

// Temporary includes for printing and FEN
#include <iostream>
#include <bitset>
#include <string>


// Include for benchmarking
#include <chrono>

// Include for testing
#include <vector>
#include <algorithm>

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

static constexpr int MOVE_SIZE = 6; 
static constexpr uint64_t HOMESQUARE_B = 1ULL << 4;
static constexpr uint64_t HOMESQUARE_R = 1ULL << 45;

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


static constexpr int SHIFTS[2][MOVE_SIZE] = {
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



//------------------------------------------------------------------------------------------------------------
static void clear_board(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR) {
    std::memset(moves, 0, sizeof(moves)); // Clear the moves array
    std::memset(figuresB, 0, sizeof(figuresB)); // Clear the figuresB array
    std::memset(figuresR, 0, sizeof(figuresR)); // Clear the figuresR array
    std::memset(figuresB_2d, 0, sizeof(figuresB_2d)); // Clear the figuresB_2d array
    std::memset(figuresR_2d, 0, sizeof(figuresR_2d)); // Clear the figuresR_2d array
    guardB = 0; // Clear the guardB variable
    guardR = 0; // Clear the guardR variable
}

static void set_board(std::string fen_pos, uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn) {
    // no handling of  invalid FEN strings
    clear_board(moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR); // Clear the board
    std::string rows[7];
    int counter = 0;
    int l = 0;
    size_t start = 0;
    size_t end = fen_pos.find('/');
    while (end != std::string::npos) {
        rows[counter++] = fen_pos.substr(start, end - start);
        start = end + 1;
        end = fen_pos.find('/', start);
    }
    rows[counter] = fen_pos.substr(start, fen_pos.find(' ') - start);

    size_t turn_pos = fen_pos.find(' ') + 1;
    isBlueTurn = (fen_pos[turn_pos] == 'b');


    for (int i = 0; i < 7; ++i) {
        int st_ind = 0;
        int j = 0;
        while (st_ind < rows[i].length()) {
            if (isdigit(rows[i][st_ind])) {
                j+= rows[i][st_ind] - '0' - 1;  
            } else if (rows[i][st_ind] == 'b') {
                uint8_t height = rows[i][++st_ind] - '0';
                figuresB_2d[i*7 + j] = height; // Set the height in the 2D array
                figuresB[height - 1] |= 1ULL << (i*7 + j); // Set the bit for the figure
            } else if (rows[i][st_ind] == 'r') {
                uint8_t height = rows[i][++st_ind] - '0';
                figuresR_2d[i*7 + j] = height; // Set the height in the 2D array
                figuresR[height - 1] |= 1ULL << (i*7 + j); // Set the bit for the figure

            } else if (rows[i][st_ind] == 'B') {
                guardB |= 1ULL << (i*7 + j); // Set the bit for the guard
                // figuresB_2d[i*7 + j] = 1; // Set the height in the 2D array
                // figuresB[0] |= 1ULL << (i*7 + j);
                st_ind++;

            } else if (rows[i][st_ind] == 'R') {
                guardR |= 1ULL << (i*7 + j); // Set the bit for the guard
                // figuresR_2d[i*7 + j] = 1; // Set the height in the 2D array
                // figuresR[0] |= 1ULL << (i*7 + j);
                st_ind++;

            } else {
                std::cout << "Error: Invalid character " << rows[i][st_ind] << "in FEN string" << std::endl;
                std::cout << rows[i] << std::endl;
            }
            st_ind++;
            j++;
        }
    }
}

static void init_board(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth) {
    clear_board(moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR); // Clear the board
    figuresB[0] = 0b0000000000000000000000000000000000000000000000100000101001100011;
    figuresR[0] = 0b0000000000000001100011001010000010000000000000000000000000000000;

    guardB = 0b0000000000000000000000000000000000000000000000000000000000001000;
    guardR = 0b0000000000000000001000000000000000000000000000000000000000000000;
    
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 7; ++j) {
            if (figuresB[0] & (1ULL << (i * 7 + j))) {
            figuresB_2d[i * 7 + j] = 1; // Set the 2D array for Blue figures
            }
            if (figuresR[0] & (1ULL << (i * 7 + j))) {
            figuresR_2d[i * 7 + j] = 1; // Set the 2D array for Red figures
            }
        }
    }
    depth = 0; // Initialize depth for alpha-beta pruning
}
// works for both players, just switch R and B arguments
static void move_generation(uint64_t (&moves)[24][2], uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR) {
    int adress_count = 0; // Reset adress count for the new generation
    //uint64_t enemy_moves = 0;
    
    // Iterate over 6 possible move sizes
    #pragma GCC unroll 6
    for (int s = 0; s < 6; ++s) {   
        // load into registers for faster access
        // load into registers for faster access
        uint64_t figsB = figuresB[s];
        uint64_t clear_mask_B = ~(figuresR[s+1] | guardB);
        uint64_t shift_0 = SHIFTS[0][s];
        uint64_t shift_1 = SHIFTS[1][s];
        uint64_t mask_type = MASK_TYPE[s];

        // calculte the end positions
        uint64_t endpos_1 = (figsB & MASK_LEFT_MOVES[0][s]) << shift_0 & clear_mask_B; // LEFT SHIFTS
        uint64_t endpos_2 = (figsB & MASK_LEFT_MOVES[1][s]) << shift_1 & clear_mask_B; // LEFT SHIFTS
        uint64_t endpos_3 = (figsB & MASK_RIGHT_MOVES[0][s]) >> shift_0 & clear_mask_B; // RIGHT SHIFTS
        uint64_t endpos_4 = (figsB & MASK_RIGHT_MOVES[1][s]) >> shift_1 & clear_mask_B; // RIGHT SHIFTS

        moves[adress_count][0] = endpos_1 >> shift_0; // store the moves of the dimension in the moves array; notice if no mossives were possible startpos is 0
        moves[adress_count++][1] = endpos_1 | mask_type; // set the stack height of the moving part of the figure
        moves[adress_count][0] = endpos_2 >> shift_1; // store the moves of the dimension in the moves array; notice if no mossives were possible startpos is 0
        moves[adress_count++][1] = endpos_2 | mask_type; // set the stack height of the moving part of the figure
        moves[adress_count][0] = endpos_3 << shift_0; // store the moves of the dimension in the moves array; notice if no mossives were possible startpos is 0
        moves[adress_count++][1] = endpos_3 | mask_type; // set the stack height of the moving part of the figure
        moves[adress_count][0] = endpos_4 << shift_1; // store the moves of the dimension in the moves array; notice if no mossives were possible startpos is 0
        moves[adress_count++][1] = endpos_4 | mask_type; // set the stack height of the moving part of the figure
    }

    uint64_t shift_0 = SHIFTS[0][0];
    uint64_t shift_1 = SHIFTS[1][0];
    uint64_t mask_type = MASK_TYPE[0];
    uint64_t not_figB = ~figuresB[0];

    uint64_t endpos_1 = (guardB & MASK_LEFT_MOVES[0][0])  << shift_0 & not_figB;
    uint64_t endpos_2 = (guardB & MASK_LEFT_MOVES[1][0])  << shift_1 & not_figB;
    uint64_t endpos_3 = (guardB & MASK_RIGHT_MOVES[0][0]) >> shift_0 & not_figB;
    uint64_t endpos_4 = (guardB & MASK_RIGHT_MOVES[1][0]) >> shift_1 & not_figB;
    
    moves[0][0] |= endpos_1 >> shift_0;
    moves[0][1] |= endpos_1 | mask_type;
    moves[1][0] |= endpos_2 >> shift_1;
    moves[1][1] |= endpos_2 | mask_type;
    moves[2][0] |= endpos_3 << shift_0;
    moves[2][1] |= endpos_3 | mask_type;
    moves[3][0] |= endpos_4 << shift_1;
    moves[3][1] |= endpos_4 | mask_type;
}       

// works for both players, just switch R and B arguments
inline static bool move(uint64_t from, uint64_t to, uint64_t* __restrict figuresB, uint64_t* __restrict figuresR, uint8_t* __restrict figuresB_2d, uint8_t* figuresR_2d, uint64_t &guardB, uint64_t guardR) {
    if (__builtin_expect(guardB & from,0)) {
        // Guard Move
        guardB ^= (guardB ^ to) & -(from & guardB == 0); // x ^= (x ^ newval) & -cond; 
    } else {

        uint64_t leaving_height = (to & MASK_STACKHEIGHT) >> TYPE_INDEX;
        uint8_t origin_height  = figuresB_2d[__builtin_ctzll(from)]; // Remove the height from the origin position
        uint8_t target_height  = figuresB_2d[__builtin_ctzll(to)]; // Add the height to the destination position

        for (int l = 1; l <= leaving_height; ++l) {
            // Remove the positions from the origin position
            figuresB[origin_height - l] ^= from; 

            // Add the positions to the destination position	
            figuresB[target_height + l] |= to;
        }
        figuresB_2d[__builtin_ctzll(from)] -= leaving_height;
        figuresB_2d[__builtin_ctzll(to)] += leaving_height;
    }
    // delete enemy
    if (__builtin_expect(to & figuresR[0],0)) {
        figuresR[0] ^= ~(to & -(to & figuresR[0] == 0));  
        figuresR[1] ^= ~(to & -(to & figuresR[1] == 0));  
        figuresR[2] ^= ~(to & -(to & figuresR[2] == 0));  
        figuresR[3] ^= ~(to & -(to & figuresR[3] == 0));  
        figuresR[4] ^= ~(to & -(to & figuresR[4] == 0));  
        figuresR[5] ^= ~(to & -(to & figuresR[5] == 0));  
        figuresR[6] ^= ~(to & -(to & figuresR[6] == 0));  
        figuresR_2d[__builtin_ctzll(to)] = 0; // Remove the height from the origin position
    }
    // check winconditions --> true if won
    return (from & guardB && to & HOMESQUARE_R) || (to & guardR);
}

// TODO in caller function: while (startpos)
inline static void extract_move(uint64_t &to, uint64_t &from, uint64_t &startpos, uint64_t &endpos) {
    from = 1ULL << __builtin_ctzll(startpos) ; 
    to = 1ULL << __builtin_ctzll(endpos) | MASK_STACKHEIGHT & endpos; // Add stack height of moving part of the figure  
    startpos &= startpos - 1; // Clear the lowest set bit
    endpos &= endpos - 1; // Clear the lowest set bit
}

// Pinting and String Operations -----------------------------------------------------------------------------------
inline static std::string FEN_position(std::uint64_t pos) {
    uint8_t x = 0;
    uint8_t y = 0;
    int index = __builtin_ctzll(pos); // Get the position of the lowest set bit
    x =  7 - (index % 7); // Column
    y =  6 - (index / 7); // Row  
    char letter = 'G' - y;
    return letter + std::to_string(x); // Convert to string
}

static std::string extract_FEN_Moves(uint64_t (&moves)[24][2]) {
    std::string result = ""; // Initialize the result string

    for(int i = 0; i < 4*MOVE_SIZE; ++i) {
        if (!moves[i][0]) break; // Stop if no more moves are available
        uint64_t startpos = moves[i][0]; // Get the start position
        uint64_t endpos = moves[i][1]; // Get the end position
        uint64_t to;
        uint64_t from;
        uint64_t leaving_height = ((endpos & MASK_STACKHEIGHT) >> TYPE_INDEX); // Get the stack height of the moving part of the figure

        while(startpos) {
            extract_move(to, from, startpos, endpos);
            result += FEN_position(from) + '-' + FEN_position(to) + '-' + std::to_string(leaving_height) + ", "; // Add the move to the result string
        }
    }
    return result;
}
  

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


// Benchmarking --------------------------------------------------------------------------------------------------- 
constexpr int NUM_RUNS = 10000000; // Number of tests to run
void benchmark_generator(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int &depth) {
    volatile uint64_t dummy_0 = 0; // Dummy variable to prevent optimization
    volatile uint64_t dummy_1 = 0; // Dummy variable to prevent optimization

    auto start = std::chrono::steady_clock::now(); // Start the timer
    for (int i = 0; i < NUM_RUNS; ++i) {
        figuresB[i%7] = 1ULL << (i % 64);

        move_generation(moves[0], figuresB, figuresR, guardB, guardR);
        dummy_0 ^= moves[0][i % 24][0];
        dummy_1 ^= moves[0][i % 24][1];
    }
    auto end = std::chrono::steady_clock::now(); // Stop the timer
    std::cout << "Dummy 0: " << dummy_0 << std::endl; // Print the dummy variable
    std::cout << "Dummy 1: " << dummy_1 << std::endl; // Print the dummy variable
    std::chrono::duration<double> elapsed = end - start; // Calculate the elapsed time
    std::cout << "Time taken for " << NUM_RUNS << " runs: " << elapsed.count() << " seconds" << std::endl; // Print the elapsed time
}

void dummy_benchmark(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int &depth){

    volatile uint64_t dummy_0 = 0; // Dummy variable to prevent optimization
    volatile uint64_t dummy_1 = 0; // Dummy variable to prevent optimization
    volatile uint64_t dummy_2 = 0; // Dummy variable to prevent optimization


    auto start = std::chrono::steady_clock::now(); // Start the timer
    for (int i = 0; i < NUM_RUNS; ++i) {

        dummy_2 ^= figuresB[i%7] = 1ULL << (i % 64);

        // move_generationB(moves[0], figuresB, figuresR, guardB, guardR);
        dummy_0 ^= moves[0][i%24][0];
        dummy_1 ^= moves[0][i%24][1];
    }
    auto end = std::chrono::steady_clock::now(); // Stop the timer
    std::cout << "Dummy 0: " << dummy_0 << std::endl; // Print the dummy variable
    std::cout << "Dummy 1: " << dummy_1 << std::endl; // Print the dummy variable
    std::cout << "Dummy 2: " << dummy_2 << std::endl; // Print the dummy variable
    std::chrono::duration<double> elapsed = end - start; // Calculate the elapsed time
    std::cout << "Time taken for " << NUM_RUNS << " runs: " << elapsed.count() << " seconds" << std::endl; // Print the elapsed time
}

void test_move_generation(std::string FEN_position, std::string FEN_moves_expected, uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool isBlueTurn, int depth) {
    set_board(FEN_position,moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn); // Set the board to a specific position
    
    std::string fen_moves;
    std::cout << isBlueTurn<< std::endl;
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

int main() {  
    uint64_t figuresB[7];
    uint64_t figuresR[7];
    uint8_t figuresB_2d[49];
    uint8_t figuresR_2d[49];
    uint64_t guardB;
    uint64_t guardR;
    uint64_t moves[MAX_DEPTH][24][2];
    bool isBlueTurn = true; // Initialize the turn
    int depth; 

    test_move_generation("RGBG5/7/7/7/7/7/7 b", "A6-A5-1, A6-B6-1, A6-A7-1", moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth); // Test the move generation
    // set_board("RGBG5/7/7/7/7/7/7 r",moves, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn); // Set the board to a specific position
    
    // print_board(figuresB, figuresR, guardB, guardR); // Print the board
    // move_generationB(moves[0], figuresB, figuresR, guardB, guardR);
    // std::cout << "Generated moves: " << std::endl;
    // std::cout << extract_FEN_Moves(moves[0]) << std::endl; // Extract the FEN moves
    return 0;
}


