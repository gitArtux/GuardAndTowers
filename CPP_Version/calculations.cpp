// Inports
#include <cstdint>
#include "const.hpp"

// Exports
#include "calculations.hpp"


using namespace masks;


struct MoveStack {
    uint64_t data[MAX_DEPTH*2];
    int top = 0;
    inline void push(uint64_t m)  { data[top++] = m; }
    inline uint64_t pop()         { return data[--top]; }
};

// works for both players, just switch R and B arguments
void move_generation(uint64_t (&moves)[24][2], uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR) {
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

void undo(MoveStack stack, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB) {
    uint64_t to = stack.pop(); // Get the last move
    uint64_t from = stack.pop(); // Get the last move

    if (__builtin_expect(guardB & to,0)) {
        // Guard Move
        guardB = to; // x ^= (x ^ newval) & -cond;
        return;
    }
    
    uint64_t leaving_height = (to & MASK_STACKHEIGHT) >> TYPE_INDEX;
    uint8_t origin_height  = figuresB_2d[__builtin_ctzll(from)]; // Remove the height from the origin position
    uint8_t target_height  = figuresB_2d[__builtin_ctzll(to)]; // Add the height to the destination position
    uint8_t captured_height = (to & CAPTURE_MASK) >> CAPTURE_INDEX; // Remove the height from the origin position

    for (int l = 0; l < leaving_height; ++l) {
        // !! Meta data gets written aswell but doesnt matter	
        figuresB[target_height - l] ^= to; 

        // Add the positions to the destination position	
        figuresB[origin_height + l + 1] |= from;
    }
    figuresB_2d[__builtin_ctzll(from)] += leaving_height;
    figuresB_2d[__builtin_ctzll(to)] -= leaving_height;

    if (__builtin_expect(captured_height,0)) {
        for (int l = 0; l < captured_height; ++l) {
            // !! Meta data gets written aswell but doesnt matter	
            figuresR[l] ^= to;  
        }
        figuresR_2d[__builtin_ctzll(to)] = captured_height; // Remove the height from the origin position
    }

}

// works for both players, just switch R and B arguments
void move(MoveStack stack, uint64_t from, uint64_t to, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB) {
    if (__builtin_expect(guardB & from,0)) {
        // Guard Move
        guardB = to;
    } else {

        uint64_t leaving_height = (to & MASK_STACKHEIGHT) >> TYPE_INDEX;
        uint8_t origin_height  = figuresB_2d[__builtin_ctzll(from)]; // Remove the height from the origin position
        uint8_t target_height  = figuresB_2d[__builtin_ctzll(to)]; // Add the height to the destination position

        for (int l = 0; l < leaving_height; ++l) {
            // Remove the positions from the origin position
            figuresB[origin_height - l] ^= from; 

            // !! Meta data gets written aswell but doesnt matter	
            figuresB[target_height + l + 1] |= to;
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
        int index = __builtin_ctzll(to); 
        to |= figuresR_2d[index] << CAPTURE_INDEX; // Remove the height from the origin position
        figuresR_2d[index] = 0; // Remove the height from the origin position
    }
    // check winconditions --> true if won
    stack.push(from);
    stack.push(to); 
}

// TODO in caller function: while (startpos)
void extract_move(uint64_t &to, uint64_t &from, uint64_t &startpos, uint64_t &endpos) {
    from = 1ULL << __builtin_ctzll(startpos) ; 
    to = 1ULL << __builtin_ctzll(endpos) | MASK_STACKHEIGHT & endpos; // Add stack height of moving part of the figure  
    startpos &= startpos - 1; // Clear the lowest set bit
    endpos &= endpos - 1; // Clear the lowest set bit
}

