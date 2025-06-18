// Inports
#include <cstdint>
#include "const.hpp"
#include <vector>
#include <iostream>


// Exports
#include "calculations.hpp"



using namespace masks;

inline void save_to_stack(Moves &moves, uint64_t startpos, uint64_t endpos) {
    while(startpos) {
        uint64_t from = 1ULL << __builtin_ctzll(startpos); 
        uint64_t to = 1ULL << __builtin_ctzll(endpos) | MASK_STACKHEIGHT & endpos; // Add stack height of moving part of the figure  
        startpos &= startpos - 1; // Clear the lowest set bit
        endpos &= endpos - 1; // Clear the lowest set bit
        moves.emplace_back(std::array<uint64_t, 2>{from, to});
    }
}

inline void save_guard_to_stack(Moves &moves, uint64_t from, uint64_t to) {
    if(to) moves.emplace_back(std::array<uint64_t, 2>{from, to | MASK_1});
}

Moves move_generation(uint64_t(&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint64_t guardB, uint64_t guardR) {
    Moves moves;
    moves.reserve(32); // Reserve space for 32 moves
    uint64_t blocked_1 = ONES;
    uint64_t blocked_2 = ONES;
    uint64_t blocked_3 = ONES;
    uint64_t blocked_4 = ONES;
    const uint64_t blocks = figuresB[0] | figuresR[0] | guardB | guardR; // Blocked positions 

    uint64_t figs = figuresB[0]; // Get the figures of player B
    int i = 0;
    uint64_t figsB = figuresB[0];
    while (figs && i < 6) {
        uint64_t figsB = figuresB[i];   
        uint64_t figsR = figuresR[i];
        uint64_t clear_mask_B = ~(figuresR[i+1] | guardB);
        uint64_t shift_0 = SHIFTS[0][i];
        uint64_t shift_1 = SHIFTS[1][i];
        uint64_t mask_type = MASK_TYPE[i];
        uint64_t mask1 = blocked_1 & clear_mask_B;
        uint64_t mask2 = blocked_2 & clear_mask_B;
        uint64_t mask3 = blocked_3 & clear_mask_B;
        uint64_t mask4 = blocked_4 & clear_mask_B;

        // calculte the end positions
        uint64_t endpos_1 = (figsB & MASK_LEFT_MOVES[0][i])  << shift_0 & mask1;
        uint64_t endpos_2 = (figsB & MASK_LEFT_MOVES[1][i])  << shift_1 & mask2;
        uint64_t endpos_3 = (figsB & MASK_RIGHT_MOVES[0][i]) >> shift_0 & mask3; 
        uint64_t endpos_4 = (figsB & MASK_RIGHT_MOVES[1][i]) >> shift_1 & mask4;

        save_to_stack(moves, endpos_1 >> shift_0, endpos_1 | mask_type);
        blocked_1 &= -static_cast<uint64_t>((endpos_1 & blocks) == 0);
        
        save_to_stack(moves, endpos_1 >> shift_0, endpos_1 | mask_type);
        blocked_2 &= -static_cast<uint64_t>((endpos_2 & blocks) == 0);

        save_to_stack(moves, endpos_1 >> shift_0, endpos_1 | mask_type);
        blocked_3 &= -static_cast<uint64_t>((endpos_3 & blocks) == 0);

        save_to_stack(moves, endpos_1 >> shift_0, endpos_1 | mask_type);
        blocked_4 &= -static_cast<uint64_t>((endpos_4 & blocks) == 0); 

        ++i;
        figsB = figuresB[i];
    }

    uint64_t shift_0  = SHIFTS[0][0];
    uint64_t shift_1  = SHIFTS[1][0];
    uint64_t not_figB = ~figuresB[0];

    save_guard_to_stack(moves, guardB, ((guardB & MASK_LEFT_MOVES[0][0])  << shift_0 & not_figB));
    save_guard_to_stack(moves, guardB, ((guardB & MASK_LEFT_MOVES[1][0])  << shift_1 & not_figB));
    save_guard_to_stack(moves, guardB, ((guardB & MASK_RIGHT_MOVES[0][0]) >> shift_0 & not_figB));
    save_guard_to_stack(moves, guardB, ((guardB & MASK_RIGHT_MOVES[1][0]) >> shift_1 & not_figB));

    
    return moves;
}


void undo(Move_History_Stack stack, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB) {
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
void move(Move_History_Stack stack, uint64_t from, uint64_t to, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB) {
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
        to |= static_cast<uint64_t>(figuresR_2d[index]) << CAPTURE_INDEX; // Remove the height from the origin position
        figuresR_2d[index] = 0; // Remove the height from the origin position
    }
    // check winconditions --> true if won
    stack.push(from);
    stack.push(to); 
}




uint64_t hit_moves(uint64_t (&figuresE)[7], uint64_t (&figuresP)[7], uint64_t guardE, uint64_t guardP) {
    uint64_t hit_moves = 0;
    uint64_t blocked_1 = ONES;
    uint64_t blocked_2 = ONES;
    uint64_t blocked_3 = ONES;
    uint64_t blocked_4 = ONES;
    uint64_t blocks = figuresE[0] | figuresP[0] | guardE | guardP; // Blocked positions 

    #pragma GCC unroll 6
    for (int s = 0; s < 6; ++s) {   
        // load into registers for faster access
        
        uint64_t figsE = figuresE[s];
        uint64_t shift_0 = SHIFTS[0][s];
        uint64_t shift_1 = SHIFTS[1][s];

        // calculte the end positions
        uint64_t endpos_1 = (figsE & MASK_LEFT_MOVES[0][s]) << shift_0 & blocked_1 & ~figuresP[s+1];
        hit_moves |= endpos_1;
        blocked_1 &= -static_cast<uint64_t>((endpos_1 & blocks) == 0);

        uint64_t endpos_2 = (figsE & MASK_LEFT_MOVES[1][s]) << shift_1 & blocked_2 & ~figuresP[s+1];
        hit_moves |= endpos_2;
        blocked_2 &= -static_cast<uint64_t>((endpos_2 & blocks) == 0);

        uint64_t endpos_3 = (figsE & MASK_RIGHT_MOVES[0][s]) >> shift_0 & blocked_3 & ~figuresP[s+1];; 
        hit_moves |= endpos_3;
        blocked_3 &= -static_cast<uint64_t>((endpos_3 & blocks) == 0);

        uint64_t endpos_4 = (figsE & MASK_RIGHT_MOVES[1][s]) >> shift_1 & blocked_4 & ~figuresP[s+1];; 
        hit_moves |= endpos_4;
        blocked_4 &= -static_cast<uint64_t>((endpos_4 & blocks) == 0);
    
    }
    uint64_t shift_0  = SHIFTS[0][0];
    uint64_t shift_1  = SHIFTS[1][0];
    hit_moves |= guardE << shift_0;
    hit_moves |= guardE << shift_1;
    hit_moves |= guardE >> shift_0;
    hit_moves |= guardE >> shift_1;

    return hit_moves; 
}


// Eval without useage of enemy use hits
int evaluate(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR) {
    int score = 0;
    // Tweek these
    // Figuere Difference
    int figure_diff = 0;
    int stack_height_B_0 = __builtin_popcountll(figuresB[0]);
    figure_diff += stack_height_B_0; 
    int stack_height_B_1 = __builtin_popcountll(figuresB[1]);
    figure_diff += stack_height_B_1; 
    int stack_height_B_2 = __builtin_popcountll(figuresB[2]);
    figure_diff += stack_height_B_2; 
    int stack_height_B_3 = __builtin_popcountll(figuresB[3]);
    figure_diff += stack_height_B_3; 
    int stack_height_B_4 = __builtin_popcountll(figuresB[4]);
    figure_diff += stack_height_B_4; 
    int stack_height_B_5 = __builtin_popcountll(figuresB[5]);
    figure_diff += stack_height_B_5; 
    int stack_height_B_6 = __builtin_popcountll(figuresB[6]);
    figure_diff += stack_height_B_6; 

    int stack_height_R_0 = - __builtin_popcountll(figuresR[6]);
    figure_diff += stack_height_R_0;
    int stack_height_R_1 = - __builtin_popcountll(figuresR[0]);
    figure_diff += stack_height_R_1;
    int stack_height_R_2 = - __builtin_popcountll(figuresR[1]);
    figure_diff += stack_height_R_2;
    int stack_height_R_3 = - __builtin_popcountll(figuresR[2]);
    figure_diff += stack_height_R_3;
    int stack_height_R_4 = - __builtin_popcountll(figuresR[3]);
    figure_diff += stack_height_R_4;
    int stack_height_R_5 = - __builtin_popcountll(figuresR[4]);
    figure_diff += stack_height_R_5;
    int stack_height_R_6 = - __builtin_popcountll(figuresR[5]);
    figure_diff += stack_height_R_6;

    
    score += 200 * figure_diff; // TODO: Tweek this value

    // Stack Height Values
    score -= 10 * (stack_height_B_0-stack_height_R_0); 
    score += 20 * (stack_height_B_1-stack_height_R_1);
    score += 10 * (stack_height_B_2-stack_height_R_2);
    score += 40 * (stack_height_B_3-stack_height_R_3);

    // higher stacks already penalized by having fewer lower stacks

  

    // positional values
    score -= 10 *(stack_height_B_1 & ROW_1);
    // score += c *(stack_height_B_1 & ROW_2);
    score += 40 *(stack_height_B_1 & ROW_3);
    score += 10 *(stack_height_B_1 & ROW_4);
    score += 80 *(stack_height_B_1 & ROW_5);
    score += 30 *(stack_height_B_1 & ROW_6);
    score += 10 *(stack_height_B_1 & ROW_7);

    score -= 10 *(stack_height_B_2 & COL_A);
    score += 30 *(stack_height_B_2 & COL_B);
    // score += 10 *(stack_height_B_2 & COL_C);
    score += 40 *(stack_height_B_2 & COL_D);
    // score += 10 *(stack_height_B_2 & COL_E);
    score += 30 *(stack_height_B_2 & COL_F);
    score -= 10 *(stack_height_B_2 & COL_G);

    return score;
}




