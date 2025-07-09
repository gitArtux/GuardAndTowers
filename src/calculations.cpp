#include <cstdint>

#include <vector>


#include "const.hpp"
#include "calculations.hpp"

using namespace masks;




Moves move_generation(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint64_t guardB, uint64_t guardR) {
    Moves moves;
    moves.reserve(32);
    
    const uint64_t blocks = figuresB[0] | figuresR[0] | guardB | guardR;

    uint64_t figs = figuresB[0];

    while (figs){
        int index = __builtin_ctzll(figs); // Get the position of the lowest set bit
        uint64_t fig = 1ULL << index; // Get the figure
        int height = figuresB_2d[index]; // Get the height of the figure

        uint64_t blocked_1 = ONES;
        uint64_t blocked_2 = ONES;
        uint64_t blocked_3 = ONES;
        uint64_t blocked_4 = ONES;

        for(int h=0; h<height; ++h){
            uint64_t clear_mask_B = ~(figuresR[h+1] | guardB);
            uint64_t shift_0 = SHIFTS[0][h];
            uint64_t shift_1 = SHIFTS[1][h];

            // calculte the end positions
            uint64_t endpos_1 = (fig & MASK_LEFT_MOVES[0][h])  << shift_0 & blocked_1;
            uint64_t endpos_2 = (fig & MASK_LEFT_MOVES[1][h])  << shift_1 & blocked_2;
            uint64_t endpos_3 = (fig & MASK_RIGHT_MOVES[0][h]) >> shift_0 & blocked_3; 
            uint64_t endpos_4 = (fig & MASK_RIGHT_MOVES[1][h]) >> shift_1 & blocked_4;

            blocked_1 &= -static_cast<uint64_t>((endpos_1 & blocks) == 0);
            blocked_2 &= -static_cast<uint64_t>((endpos_2 & blocks) == 0);
            blocked_3 &= -static_cast<uint64_t>((endpos_3 & blocks) == 0);
            blocked_4 &= -static_cast<uint64_t>((endpos_4 & blocks) == 0);

            endpos_1 &= clear_mask_B;
            endpos_2 &= clear_mask_B;
            endpos_3 &= clear_mask_B;
            endpos_4 &= clear_mask_B;

            uint64_t mask_type = MASK_TYPE[h];
            if (endpos_1) moves.emplace_back(std::array<uint64_t, 2>{fig, endpos_1 | mask_type});
            if (endpos_2) moves.emplace_back(std::array<uint64_t, 2>{fig, endpos_2 | mask_type});
            if (endpos_3) moves.emplace_back(std::array<uint64_t, 2>{fig, endpos_3 | mask_type});
            if (endpos_4) moves.emplace_back(std::array<uint64_t, 2>{fig, endpos_4 | mask_type});
        }
        figs &= figs - 1; // Clear the lowest set bit
    }

    uint64_t shift_0  = SHIFTS[0][0];
    uint64_t shift_1  = SHIFTS[1][0];
    uint64_t not_figB = ~figuresB[0];
    uint64_t dests = 0;
    uint64_t endpos_1 = (guardB & MASK_LEFT_MOVES[0][0])  << shift_0 & not_figB;
    uint64_t endpos_2 = (guardB & MASK_LEFT_MOVES[1][0])  << shift_1 & not_figB;
    uint64_t endpos_3 = (guardB & MASK_RIGHT_MOVES[0][0]) >> shift_0 & not_figB;
    uint64_t endpos_4 = (guardB & MASK_RIGHT_MOVES[1][0]) >> shift_1 & not_figB;

    if (endpos_1) moves.emplace_back(std::array<uint64_t, 2>{guardB, endpos_1 | MASK_1});
    if (endpos_2) moves.emplace_back(std::array<uint64_t, 2>{guardB, endpos_2 | MASK_1});
    if (endpos_3) moves.emplace_back(std::array<uint64_t, 2>{guardB, endpos_3 | MASK_1});
    if (endpos_4) moves.emplace_back(std::array<uint64_t, 2>{guardB, endpos_4 | MASK_1});

    return moves;
}





// works for both players, just switch R and B arguments
void do_move(Move &move, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR) {
    uint64_t from = move[0];
    uint64_t to   = move[1];
    // history.emplace_back(std::array<uint64_t, 2>{from, to});


    uint64_t leaving_height = (to & MASK_STACKHEIGHT) >> TYPE_INDEX;
    to &= FILTER_FIG; // Remove the type from the destination position

    // hit guard
    guardR ^= guardR & to;

    if (guardB & from) {
        // Guard Move
        guardB = to;
    } else {
        int index= __builtin_ctzll(from);
        uint8_t origin_height  = figuresB_2d[__builtin_ctzll(from)]; // Remove the height from the origin position
        uint8_t target_height  = figuresB_2d[__builtin_ctzll(to)]; // Add the height to the destination position
        
        for (int l = 0; l < leaving_height; ++l) {
            
            // Remove the positions from the origin position
            figuresB[origin_height - l - 1] ^= from; 

            // !! Meta data gets written aswell but doesnt matter	
            figuresB[target_height + l] |= to;
        }
        figuresB_2d[__builtin_ctzll(from)] -= leaving_height;
        figuresB_2d[__builtin_ctzll(to)] += leaving_height;
    }
    // delete enemy
    if (to & figuresR[0]) {
        figuresR[0] ^= to;
        for (int h = 1; h < 7; ++h) {
            if (to & figuresR[h]) {
                figuresR[h] ^= to; // Remove the captured figure
            } else {
                break; // No more figures to remove
            }
        } 
        int index = __builtin_ctzll(to); 
        move[1] |= static_cast<uint64_t>(figuresR_2d[index]) << CAPTURE_INDEX; // Remove the height from the origin position
        figuresR_2d[index] = 0; // Remove the height from the origin position
    }
}

void undo(Move move, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR) {
    uint64_t from = move[0];
    uint64_t to   = move[1]; // Remove the type from the destination position
    uint64_t leaving_height = (to & MASK_STACKHEIGHT) >> TYPE_INDEX;
    uint8_t captured_height = (to & CAPTURE_MASK) >> CAPTURE_INDEX;
    to &= FILTER_FIG; // Remove the type from the destination position
     // Remove the height from the origin position
    
    if (captured_height) {
            for (int l = 0; l < captured_height; ++l) {
                // !! Meta data gets written aswell but doesnt matter	
                figuresR[l] ^= to;  
            }
        figuresR_2d[__builtin_ctzll(to)] = captured_height; // Remove the height from the origin position
    }

    // bring guard back to live
    guardR |= -!guardR & to;


    if (guardB & to) {
        // Guard Move
        guardB = from; // x ^= (x ^ newval) & -cond;
        
    } else {
    
        int index = __builtin_ctzll(from); // Get the index of the lowest set bit
        
        uint8_t origin_height  = figuresB_2d[__builtin_ctzll(from)]; // Remove the height from the origin position
        uint8_t target_height  = figuresB_2d[__builtin_ctzll(to)]; // Add the height to the destination position

        for (int l = 0; l < leaving_height; ++l) {
            // !! Meta data gets written aswell but doesnt matter	
            figuresB[target_height - l - 1] ^= to; 

            // Add the positions to the destination position	
            figuresB[origin_height + l] |= from;
        }
        figuresB_2d[__builtin_ctzll(from)] += leaving_height;
        figuresB_2d[__builtin_ctzll(to)] -= leaving_height;
    }

}

