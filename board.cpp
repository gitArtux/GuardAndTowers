#include <cstdint>

// Temporary includes for printing
#include <iostream>


//------------------------------------------



/*
TODO: Think about return types for performance
TODO: Think about static class vs no class
TODO: Think about C-style array vs std::array 

TODO: carful to not use vectorization (SIMD) due to overhead
*/


class Board {
    public:
        static constexpr int MAX_DEPTH = 8; // Max depth for alpha-beta pruning
        static constexpr int MAX_MOVES = 32;

    private:
        // BOARD REPRESENTATION -----------------------------------------------------------------------------------------------
        static uint64_t figuresB;
        static uint64_t figuresR;

        // guards are on index 0
        alignas(64) static uint64_t l_figuresB[8];
        alignas(64) static uint64_t l_figuresR[8];

        //------------------------------------------------------------------------------------------------------------



        // ALPHA-BETA PRUNING -----------------------------------------------------------------------------------------------
        // TODO: define MAX_DEAPTH
        // TODO: C-style array or std::array?
        
        static int depth;
        alignas(64) static uint64_t moves[MAX_DEPTH][MAX_MOVES];
        static uint64_t move_stack[MAX_DEPTH][2];
        //------------------------------------------------------------------------------------------------------------

        
        // SHIFTS -------------------------------------------------------------------------------------------------------------------
        // TODO: shifts for move calculation

        //-------------------------------------------------------------------------------------------------------------------------

        // MASKS -------------------------------------------------------------------------------------------------------------------
        // TODO: set up the masks for the move calculation
        // TODO: reorganize for the different dimensions for early stoppings
        static constexpr uint64_t MASK_1RIGHT = 0b000000000000000111111011111101111110111111011111101111110;
        static constexpr uint64_t MASK_1LEFT  = 0b000000000000000011111101111110111111011111101111110111111;
        static constexpr uint64_t MASK_1UP    = 0b000000000000000000000011111111111111111111111111111111111;
        static constexpr uint64_t MASK_1DOWN  = 0b000000000000000111111111111111111111111111111111111111111;

        static constexpr uint64_t MASK_2RIGHT = 0b000000000000000111110011111001111100111110011111001111100;
        static constexpr uint64_t MASK_2LEFT  = 0b000000000000000001111101111110111111011111101111110111111;
        static constexpr uint64_t MASK_2UP    = 0b000000000000000000000000000000111111111111111111111111111;
        static constexpr uint64_t MASK_2DOWN  = 0b000000000000000111111111111111111111111111111111000000000;

        static constexpr uint64_t MASK_3RIGHT = 0b000000000000000111100011110001111000111100011110001111000;
        static constexpr uint64_t MASK_3LEFT  = 0b000000000000000000111101111110111111011111101111110111111;
        static constexpr uint64_t MASK_3UP    = 0b000000000000000000000000000000000000011111111111111111111;
        static constexpr uint64_t MASK_3DOWN  = 0b000000000000000111111111111111111111000000000000000000000;

        static constexpr uint64_t MASK_4RIGHT = 0b000000000000000111000011100001110000111000011100001110000;
        static constexpr uint64_t MASK_4LEFT  = 0b000000000000000000011101111110111111011111101111110111111;
        static constexpr uint64_t MASK_4UP    = 0b000000000000000000000000000000000000000000111111111111111;
        static constexpr uint64_t MASK_4DOWN  = 0b000000000000000111111111111111000000000000000000000000000;

        static constexpr uint64_t MASK_5RIGHT = 0b000000000000000110000011000001100000110000011000001100000;
        static constexpr uint64_t MASK_5LEFT  = 0b000000000000000000001101111110111111011111101111110111111;
        static constexpr uint64_t MASK_5UP    = 0b000000000000000000000000000000000000000000000000111111111;
        static constexpr uint64_t MASK_5DOWN  = 0b000000000000000111111111111111000000000000000000000000000;

        static constexpr uint64_t MASK_6RIGHT = 0b000000000000000100000010000001000000100000010000001000000;
        static constexpr uint64_t MASK_6LEFT  = 0b000000000000000000000101111110111111011111101111110111111;
        static constexpr uint64_t MASK_6UP    = 0b000000000000000000000000000000000000000000000000000000111;
        static constexpr uint64_t MASK_6DOWN  = 0b000000000000000111111100000000000000000000000000000000000;

        alignas(64) static constexpr uint64_t MASK_RIGHT[6] = {MASK_1RIGHT, MASK_2RIGHT, MASK_3RIGHT, MASK_4RIGHT, MASK_5RIGHT, MASK_6RIGHT};
        alignas(64) static constexpr uint64_t MASK_LEFT[6]  = {MASK_1LEFT,  MASK_2LEFT,  MASK_3LEFT,  MASK_4LEFT,  MASK_5LEFT,  MASK_6LEFT};
        alignas(64) static constexpr uint64_t MASK_UP[6]    = {MASK_1UP,    MASK_2UP,    MASK_3UP,    MASK_4UP,    MASK_5UP,    MASK_6UP};
        alignas(64) static constexpr uint64_t MASK_DOWN[6]  = {MASK_1DOWN,  MASK_2DOWN,  MASK_3DOWN,  MASK_4DOWN,  MASK_5DOWN,  MASK_6DOWN};

        // Figure types/height
        static constexpr uint8_t TYPE_INDEX = 61;
        static constexpr uint64_t MASK_G = 3ULL << TYPE_INDEX-1; // TODO: how to handle guard?
        static constexpr uint64_t MASK_1 = 1ULL << TYPE_INDEX; 
        static constexpr uint64_t MASK_2 = 2ULL << TYPE_INDEX; 
        static constexpr uint64_t MASK_3 = 3ULL << TYPE_INDEX; 
        static constexpr uint64_t MASK_4 = 4ULL << TYPE_INDEX; 
        static constexpr uint64_t MASK_5 = 5ULL << TYPE_INDEX; 
        static constexpr uint64_t MASK_6 = 6ULL << TYPE_INDEX; 
        static constexpr uint64_t MASK_7 = 7ULL << TYPE_INDEX; 

        alignas(64) static constexpr uint64_t MASK_TYPE[8] = {MASK_1, MASK_2, MASK_3, MASK_4, MASK_5, MASK_6, MASK_7, MASK_G};

        // Board Mask
        static constexpr uint64_t MASK_FIGURE_TYPE = 0b111ULL << TYPE_INDEX; 
        static constexpr uint64_t MASK_BOARD = (1ULL << 49) - 1;
        //------------------------------------------------------------------------------------------------------------


    public:   
        static void init_board() {
            // Reset the board to its initial state
            figuresB = 0b0000000000000000000000000000000000000000000000100000101001101011;
            figuresR = 0b0000000000000001101011001010000010000000000000000000000000000000;
            
            // guards are on index 0
            // Initialize l_figuresB
            l_figuresB[0] = 0b0011000000000000000000000000000000000000000000000000000000001000;
            l_figuresB[1] = 0b0010000000000000000000000000000000000000000000000000000000000001;
            l_figuresB[2] = 0b0010000000000000000000000000000000000000000000000000000000000010;
            l_figuresB[3] = 0b0010000000000000000000000000000000000000000000000000000000100000;
            l_figuresB[4] = 0b0010000000000000000000000000000000000000000000000000000001000000;
            l_figuresB[5] = 0b0010000000000000000000000000000000000000000000000000001000000000;
            l_figuresB[6] = 0b0010000000000000000000000000000000000000000000000000100000000000;
            l_figuresB[7] = 0b0010000000000000000000000000000000000000000000100000000000000000;

            // Initialize l_figuresR
            l_figuresR[0] = 0b0011000000000000001000000000000000000000000000000000000000000000;
            l_figuresR[1] = 0b0010000000000001000000000000000000000000000000000000000000000000;
            l_figuresR[2] = 0b0010000000000000100000000000000000000000000000000000000000000000;
            l_figuresR[3] = 0b0010000000000000000010000000000000000000000000000000000000000000;
            l_figuresR[4] = 0b0010000000000000000001000000000000000000000000000000000000000000;
            l_figuresR[5] = 0b0010000000000000000000001000000000000000000000000000000000000000;
            l_figuresR[6] = 0b0010000000000000000000000010000000000000000000000000000000000000;
            l_figuresR[7] = 0b0010000000000000000000000000000010000000000000000000000000000000;
    
                        
            depth = 0; // Initialize depth for alpha-beta pruning
        }

        static void move_generation(int depth) {
            // Implement the move generation logic here
        }

        static void move(uint64_t from, uint64_t to) {
            // Implement the move logic here
        }

        static void undo() {
            // Implement the undo logic here
        }


        static void pick_move() {
            // Implement the move picking logic here
        }

        static void print_board() {
            // Empty==0, Blue>0, Red<0
            std::string board[7][7];
            for (int i = 0; i < 7; ++i) {
                for (int j = 0; j < 7; ++j) {
                    board[i][j] = "| ";
                }
            }
            std::string type = " "; // guard=8
            uint8_t x = 0;
            uint8_t y = 0;
            // Fill the board with the figures
          
            for (int l = 0; l < 2; ++l){
                uint64_t* figures = (l == 0) ? l_figuresB : l_figuresR; // Pointer to the current player's figures
                for (int k = 0; k < 8; ++k) {
                    uint8_t pos = figures[k] ? static_cast<uint8_t>(__builtin_ctzll(figures[k])) : 0; // Use __builtin_ctzll for 64-bit integers
                    x =  6 - (pos % 7); // Column
                    y =  6 - (pos / 7); // Row  
                    for (int i = 0; i < 8; ++i) {
                        if ((figures[k] & MASK_TYPE[i]) == MASK_TYPE[i]) {
                            if (i + 1 == 8) {
                                type = "G"; // Guard
                            } else {
                                type = std::to_string(i + 1); // Convert to string
                            }
                            
                            if (l == 0) {
                                board[y][x] = "|\033[34m" + type + "\033[0m"; // Blue   
                            } else {
                                board[y][x] = "|\033[31m" + type + "\033[0m"; // Red 
                            }
                        } 
                    }
                }
            }
            // BLUE: \033[34m \033[0m
            // RED: \033[31m \033[0m
            // underline: \033[4m \033[0m

            // Print the board
            for (int i = 0; i < 7; ++i) {
                char letter = 'G' - i; // Convert index to letter
                std::cout << letter << "  "; 
                for (int j = 0; j < 7; ++j) {
                    std::cout << board[i][j]; 
                }
                std::cout << "|" << "\n";
            }
            std::cout << "\n" << "    1 2 3 4 5 6 7 " << std::endl; // Column numbers
        }
};

uint64_t Board::figuresB;
uint64_t Board::figuresR;
uint64_t Board::l_figuresB[8];
uint64_t Board::l_figuresR[8];
int Board::depth = 0;

uint64_t moves[Board::MAX_DEPTH][Board::MAX_MOVES];
uint64_t move_stack[Board::MAX_DEPTH][2];

int main() {  
    Board::init_board(); // Initialize the board
    Board::print_board(); // Print the board
    return 0;
}
