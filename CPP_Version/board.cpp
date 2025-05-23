#include "board.h"
#include "calculations.h"

#include <bitset>
#include <string>

#include <cstring> // For memset
#include <iostream>

// basic print function
void test_board_link(const std::string& str) {
    std::cout << str << std::endl;
}

void clear_board(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR) {
    std::memset(moves, 0, sizeof(moves)); // Clear the moves array
    std::memset(figuresB, 0, sizeof(figuresB)); // Clear the figuresB array
    std::memset(figuresR, 0, sizeof(figuresR)); // Clear the figuresR array
    std::memset(figuresB_2d, 0, sizeof(figuresB_2d)); // Clear the figuresB_2d array
    std::memset(figuresR_2d, 0, sizeof(figuresR_2d)); // Clear the figuresR_2d array
    guardB = 0; // Clear the guardB variable
    guardR = 0; // Clear the guardR variable
}


void init_board(uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn, int &depth) {
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


void set_board(std::string fen_pos, uint64_t (&moves)[MAX_DEPTH][24][2], uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool &isBlueTurn) {
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


// Pinting and String Operations -----------------------------------------------------------------------------------
inline std::string FEN_position(std::uint64_t pos) {
    uint8_t x = 0;
    uint8_t y = 0;
    int index = __builtin_ctzll(pos); // Get the position of the lowest set bit
    x =  7 - (index % 7); // Column
    y =  6 - (index / 7); // Row  
    char letter = 'G' - y;
    return letter + std::to_string(x); // Convert to string
}

std::string extract_FEN_Moves(uint64_t (&moves)[24][2]) {
    std::string result = ""; // Initialize the result string

    for(int i = 0; i < 4*6; ++i) {
        if (!moves[i][0]) break; // Stop if no more moves are available
        uint64_t startpos = moves[i][0]; // Get the start positions
        uint64_t endpos = moves[i][1]; // Get the end position
        uint64_t to;
        uint64_t from;
        uint64_t leaving_height = ((endpos & masks::MASK_STACKHEIGHT) >> masks::TYPE_INDEX); // Get the stack height of the moving part of the figure

        while(startpos) {
            extract_move(to, from, startpos, endpos);
            result += FEN_position(from) + '-' + FEN_position(to) + '-' + std::to_string(leaving_height) + " "; // Add the move to the result string
        }
    }
    return result;
}
  

void print_board(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR) {
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
   
void print_Bitboard(std::uint64_t bitboard) {
    std::cout << "Bitboard: " << std::bitset<64>(bitboard) << std::endl; // Print the bitboard in binary format
}
