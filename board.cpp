#include <cstdint>

// Temporary includes for printing and FEN
#include <iostream>
#include <bitset>





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

// BOARD REPRESENTATION -----------------------------------------------------------------------------------------------
static std::uint64_t figuresB[7]; // set meta 0 ! --> set only the positions
static std::uint64_t figuresR[7]; // set meta 0 ! --> set only the positions
static std::uint64_t guardB; 
static std::uint64_t guardR; 

static std::uint64_t startpos; // Remaining startpos // ! set meta 0 ! --> set only the positions
static std::uint64_t endpos; // Remaining endpos, contains in meta the stack height
static std::uint64_t leaving_height; // Temporary variable for the current player

static std::uint64_t from; // Only 1 figure
static std::uint64_t to; // Only 1 figure

static constexpr std::uint64_t HOMESQUARE_B = 1ULL << 4;
static constexpr std::uint64_t HOMESQUARE_R = 1ULL << 45;




//------------------------------------------------------------------------------------------------------------

// ALPHA-BETA PRUNING -----------------------------------------------------------------------------------------------
// TODO: define MAX_DEAPTH
// TODO: C-style array or std::array?

static int depth;
alignas(64) static std::uint64_t moves[MAX_DEPTH][MOVE_DIMENSION*MOVE_SIZE][2];
static int adress_count;
static std::uint64_t move_stack[MAX_DEPTH][2];
//------------------------------------------------------------------------------------------------------------


// SHIFTS -------------------------------------------------------------------------------------------------------------------
// TODO: shifts for move calculation

//-------------------------------------------------------------------------------------------------------------------------

// MASKS -------------------------------------------------------------------------------------------------------------------
// TODO: set up the masks for the move calculation
// TODO: reorganize for the different dimensions for early stoppings
static constexpr std::uint64_t MASK_1RIGHT = 0b0000000000000001111110111111011111101111110111111011111101111110;
static constexpr std::uint64_t MASK_1LEFT  = 0b0000000000000000111111011111101111110111111011111101111110111111;
static constexpr std::uint64_t MASK_1UP    = 0b0000000000000000000000111111111111111111111111111111111111111111;
static constexpr std::uint64_t MASK_1DOWN  = 0b0000000000000001111111111111111111111111111111111111111110000000;
static constexpr std::uint64_t MASK_2RIGHT = 0b0000000000000001111100111110011111001111100111110011111001111100;
static constexpr std::uint64_t MASK_2LEFT  = 0b0000000000000000011111001111100111110011111001111100111110011111;
static constexpr std::uint64_t MASK_2UP    = 0b0000000000000000000000000000011111111111111111111111111111111111;
static constexpr std::uint64_t MASK_2DOWN  = 0b0000000000000001111111111111111111111111111111111100000000000000;
static constexpr std::uint64_t MASK_3RIGHT = 0b0000000000000001111000111100011110001111000111100011110001111000;
static constexpr std::uint64_t MASK_3LEFT  = 0b0000000000000000001111000111100011110001111000111100011110001111;
static constexpr std::uint64_t MASK_3UP    = 0b0000000000000000000000000000000000000111111111111111111111111111;
static constexpr std::uint64_t MASK_3DOWN  = 0b0000000000000001111111111111111111111111111000000000000000000000;
static constexpr std::uint64_t MASK_4RIGHT = 0b0000000000000001110000111000011100001110000111000011100001110000;
static constexpr std::uint64_t MASK_4LEFT  = 0b0000000000000000000111000011100001110000111000011100001110000111;
static constexpr std::uint64_t MASK_4UP    = 0b0000000000000000000000000000000000000000000111111111111111111111;
static constexpr std::uint64_t MASK_4DOWN  = 0b0000000000000001111111111111111111110000000000000000000000000000;
static constexpr std::uint64_t MASK_5RIGHT = 0b0000000000000001100000110000011000001100000110000011000001100000;
static constexpr std::uint64_t MASK_5LEFT  = 0b0000000000000000000011000001100000110000011000001100000110000011;
static constexpr std::uint64_t MASK_5UP    = 0b0000000000000000000000000000000000000000000000000011111111111111;
static constexpr std::uint64_t MASK_5DOWN  = 0b0000000000000001111111111111100000000000000000000000000000000000;
static constexpr std::uint64_t MASK_6RIGHT = 0b0000000000000001000000100000010000001000000100000010000001000000;
static constexpr std::uint64_t MASK_6LEFT  = 0b0000000000000000000001000000100000010000001000000100000010000001;
static constexpr std::uint64_t MASK_6UP    = 0b0000000000000000000000000000000000000000000000000000000001111111;
static constexpr std::uint64_t MASK_6DOWN  = 0b0000000000000001111111000000000000000000000000000000000000000000;



// MOVE MASKS --------------------------------------------------------------------------------------------------

static constexpr std::uint64_t MASK_LEFT_MOVES[MOVE_DIMENSION][MOVE_SIZE] = {
    {MASK_1LEFT, MASK_2LEFT, MASK_3LEFT, MASK_4LEFT, MASK_5LEFT, MASK_6LEFT},
    {MASK_1UP, MASK_2UP, MASK_3UP, MASK_4UP, MASK_5UP, MASK_6UP}
};

static constexpr std::uint64_t MASK_RIGHT_MOVES[MOVE_DIMENSION][MOVE_SIZE] = {
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


 
static void init_board() {
    // Reset the board to its initial state
    figuresB[0] = 0b0000000000000000000000000000000000000000000000100000101001100011;
    figuresB[1] = 0;
    figuresB[2] = 0;
    figuresB[3] = 0;    
    figuresB[4] = 0;
    figuresB[5] = 0;
    figuresB[6] = 0;

    figuresR[0] = 0b0000000000000001100011001010000010000000000000000000000000000000;
    figuresR[1] = 0;
    figuresR[2] = 0;
    figuresR[3] = 0;
    figuresR[4] = 0;
    figuresR[5] = 0;
    figuresR[6] = 0;


    guardB = 0b0000000000000000000000000000000000000000000000000000000000001000;
    guardR = 0b0000000000000000001000000000000000000000000000000000000000000000;
                
    depth = 0; // Initialize depth for alpha-beta pruning
}

static void move_generationB() {
    adress_count = 0; // Reset adress count for the new generation
    
    // LEFT MOVES ---------------------------------------------
    // Itereate over 4 dimensions
    for (int d = 0; d < MOVE_DIMENSION-2; ++d) {   
        // For Figures B
        // Iterate over 6 possible move sizes
        for (int s = 0; s < MOVE_SIZE; ++s) {

            // Check wether figure with step size exists
            if (figuresB[s]){
                // select figuresB with step size s and calc enpos
                startpos = figuresB[s] & MASK_LEFT_MOVES[d][s];

                // check if any figures in mask
                if (!startpos) break;
        
                // create endpos
                endpos = startpos << SHIFTS[d][s];
                
                // check for higher enemy stacks and the own guard on endpos
                endpos &= ~(endpos & (figuresR[s+1] | guardB));

                // check for empty moves
                if (endpos){
                    startpos = endpos >> SHIFTS[d][s]; // set the startpos accordingly to the endpos 

                    // store the move in the moves array
                    moves[depth][adress_count][0] = startpos; 
                    moves[depth][adress_count][1] = endpos | MASK_TYPE[s]; // set the stack height of the moving part of the figure
                    ++adress_count;
                }

                // stop criteria for blocked dimension
                else break;

            }

            // stop criteria for non existing stack height
            else break;
        }
       
    }

    // RIGHT MOVES ---------------------------------------------    
    // Itereate over 4 dimensions
    for (int d = 0; d < MOVE_DIMENSION-2; ++d) {   
        // For Figures B
        // Iterate over 6 possible move sizes
        for (int s = 0; s < MOVE_SIZE; ++s) {

            // Check wether figure with step size exists
            if (figuresB[s]){
                // select figuresB with step size s and calc enpos
                startpos = figuresB[s] & MASK_RIGHT_MOVES[d][s];

                // check if any figures in mask
                if (!startpos) break;
        
                // create endpos
                endpos = startpos >> SHIFTS[d][s];
                
                // check for higher enemy stacks and the own guard on endpos
                endpos &= ~(endpos & (figuresR[s+1] | guardB));

                // check for empty moves
                if (endpos){
                    startpos = endpos << SHIFTS[d][s]; // set the startpos accordingly to the endpos 

                    // store the move in the moves array
                    moves[depth][adress_count][0] = startpos; 
                    moves[depth][adress_count][1] = endpos | MASK_TYPE[s]; // set the stack height of the moving part of the figure
                    ++adress_count ;
                }

                // stop criteria for blocked dimension
                else break;

            }

            // stop criteria for non existing stack height
            else break;
        }
        
    }

    
}

static void move_generationR() {
    adress_count = 0; // Reset adress count for the new generation
    
    // LEFT MOVES ---------------------------------------------
    // Itereate over 4 dimensions
    for (int d = 0; d < MOVE_DIMENSION-2; ++d) {   
        // For Figures R
        // Iterate over 6 possible move sizes
        for (int s = 0; s < MOVE_SIZE; ++s) {

            // Check wether figure with step size exists
            if (figuresR[s]){
                // select figuresR with step size s and calc enpos
                startpos = figuresR[s] & MASK_LEFT_MOVES[d][s];

                // check if any figures in mask
                if (!startpos) break;
        
                // create endpos
                endpos = startpos << SHIFTS[d][s];
                
                // check for higher enemy stacks and the own guard on endpos
                endpos &= ~(endpos & (figuresB[s+1] | guardR));

                // check for empty moves
                if (endpos){
                    startpos = endpos >> SHIFTS[d][s]; // set the startpos accordingly to the endpos 

                    // store the move in the moves array
                    moves[depth][adress_count][0] = startpos; 
                    moves[depth][adress_count][1] = endpos | MASK_TYPE[s]; // set the stack height of the moving part of the figure
                    ++adress_count;
                }

                // stop criteria for blocked dimension
                else break;

            }

            // stop criteria for non existing stack height
            else break;
        }
       
    }

    // RIGHT MOVES ---------------------------------------------    
    // Itereate over 4 dimensions
    for (int d = 0; d < MOVE_DIMENSION-2; ++d) {   
        // For Figures R
        // Iterate over 6 possible move sizes
        for (int s = 0; s < MOVE_SIZE; ++s) {

            // Check wether figure with step size exists
            if (figuresR[s]){
                // select figuresR with step size s and calc enpos
                startpos = figuresR[s] & MASK_RIGHT_MOVES[d][s];

                // check if any figures in mask
                if (!startpos) break;

                // create endpos
                endpos = startpos >> SHIFTS[d][s];

                // check for higher enemy stacks and the own guard on endpos
                endpos &= ~(endpos & (figuresB[s+1] | guardR));

                //check for empty moves
                if (endpos){
                    startpos = endpos << SHIFTS[d][s]; // set the startpos accordingly to the endpos 

                    // store the move in the moves array
                    moves[depth][adress_count][0] = startpos; 
                    moves[depth][adress_count][1] = endpos | MASK_TYPE[s]; // set the stack height of the moving part of the figure
                    ++adress_count;
                }

                // stop criteria for blocked dimension
                else break;
            }

            // stop criteria for non existing stack height
            else break;
        }
    }
}

inline static bool moveB() {
    // Check Wincondition Opponent guard hit
    if (to & guardR) return true;

    // Move guard
    if (from & guardB) {
        // Check Wincondition Guard on opponent home square
        if (to & HOMESQUARE_R) return true; 
        guardB = to; // Move the guard to the new position (guard has height 1)
    }
    else {
        leaving_height = ((to & MASK_STACKHEIGHT) >> TYPE_INDEX); 

        // Remove the positions from the origin position
        for (int i = 0; i < 7; ++i) {
            if (!(from & figuresB[i])) {
                for (int l = 1; l <= leaving_height; ++l) {
                    figuresB[i-l] &= ~from; 
                }
                break;
            }
        }

        // Add the positions to the destination position
        to = to & MASK_BOARD; // Remove the stack height from the to position
        for (int i = 0; i < 7; ++i) {
            if (!(to & figuresB[i])) {
                for (int l = 0; l < leaving_height; ++l) {
                    figuresB[i+l] |= to; 
                }
                break;
            }
        }
    }
    // Remove enemy figure 
    if (to & figuresR[0]) {
        for (int i = 0; i < 7; ++i) {
            if (!(figuresR[i] & to)) break;
            figuresR[i] &= ~to;
        }
    }
    return false;
}

inline static bool moveR() {
    // Check Wincondition Opponent guard hit
    if (to & guardB) return true;

    // Move guard
    if (from & guardR) {
        // Check Wincondition Guard on opponent home square
        if (to & HOMESQUARE_B) return true; 
        guardR = to; // Move the guard to the new position (guard has height 1)
    }
    else {
        leaving_height = ((to & MASK_STACKHEIGHT) >> TYPE_INDEX); 

        // Remove the positions from the origin position
        for (int i = 0; i < 7; ++i) {
            if (!(from & figuresR[i])) {
                for (int l = 1; l <= leaving_height; ++l) {
                    figuresR[i-l] &= ~from; 
                }
                break;
            }
        }

        // Add the positions to the destination position
        to = to & MASK_BOARD; // Remove the stack height from the to position
        for (int i = 0; i < 7; ++i) {
            if (!(to & figuresR[i])) {
                for (int l = 0; l < leaving_height; ++l) {
                    figuresR[i+l] |= to; 
                }
                break;
            }
        }
    }
    // Remove enemy figure 
    if (to & figuresB[0]) {
        for (int i = 0; i < 7; ++i) {
            if (!(figuresB[i] & to)) break;
            figuresB[i] &= ~to;
        }
    }
    return false;
}


static void undo() {
    // Implement the undo logic here
}

// TODO in caller function: while (startpos)
inline static void extract_move() {
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

static void print_FEN_Moves() {

    for(int i = 0; i < MOVE_DIMENSION*MOVE_SIZE; ++i) {
        if (!moves[depth][i][0]) break; // Stop if no more moves are available

        startpos = moves[depth][i][0]; // Get the start position
        endpos = moves[depth][i][1]; // Get the end position

        leaving_height = ((endpos & MASK_STACKHEIGHT) >> TYPE_INDEX); // Get the stack height of the moving part of the figure

        while(startpos) {
            extract_move();
            
            std::cout <<  FEN_position(from) + '-' + FEN_position(to) + '-' + std::to_string(leaving_height) << ", "; // Print the move
        }
    }
    std::cout << std::endl; // Print a new line after all moves

}
  

// Pinting and String Operations -----------------------------------------------------------------------------------
static void print_board() {
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


int main() {  
    init_board(); // Initialize the board
    print_board(); // Print the initial board state
    // move_generationB(); // Generate moves for player B
    // print_FEN_Moves(); // Print the generated moves in FEN format
    move_generationB(); // Generate moves for player B
    // print_FEN_Moves(); // Print the generated moves in FEN format
    startpos = moves[depth][0][0]; // Get the start position from the generated moves
    endpos = moves[depth][0][1]; // Get the end position from the generated moves
    extract_move(); // Extract the move from the generated moves
    print_Bitboard(from); // Print the from position in bitboard format
    print_Bitboard(to); // Print the to position in bitboard format
    moveB();
    print_board(); // Print the board after the move
    std::cin.get();
    return 0;
}
