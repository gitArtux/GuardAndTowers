#include <cstdint>

#include <vector>


#include "const.hpp"

using namespace masks;


std::vector<std::array<uint64_t,2>> exp_move_gen(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t (&b_figuresB)[7], uint64_t (&b_figuresR)[7],uint64_t guardB, uint64_t guardR) {
    std::vector<std::array<uint64_t,2>> moves;
    
    const uint64_t blocks = figuresB[0] | figuresR[0] | guardB | guardR;

    for (uint64_t fig: figuresB) {
        uint64_t blocked_1 = ONES;
        uint64_t blocked_2 = ONES;
        uint64_t blocked_3 = ONES;
        uint64_t blocked_4 = ONES;
        uint64_t dests = 0;

        int height = fig >> TYPE_INDEX; // Get the type of the figure

        for(int h=1; h<=height; ++h){

            uint64_t clear_mask_B = ~(figuresR[h+1] | guardB);
            uint64_t shift_0 = SHIFTS[0][h];
            uint64_t shift_1 = SHIFTS[1][h];

            uint64_t mask1 = blocked_1 & clear_mask_B;
            uint64_t mask2 = blocked_2 & clear_mask_B;
            uint64_t mask3 = blocked_3 & clear_mask_B;
            uint64_t mask4 = blocked_4 & clear_mask_B;

            // calculte the end positions
            uint64_t endpos_1 = (fig & MASK_LEFT_MOVES[0][h])  << shift_0 & mask1;
            uint64_t endpos_2 = (fig & MASK_LEFT_MOVES[1][h])  << shift_1 & mask2;
            uint64_t endpos_3 = (fig & MASK_RIGHT_MOVES[0][h]) >> shift_0 & mask3; 
            uint64_t endpos_4 = (fig & MASK_RIGHT_MOVES[1][h]) >> shift_1 & mask4;

            dests |= endpos_1;
            blocked_1 &= -static_cast<uint64_t>((endpos_1 & blocks) == 0);
            dests |= endpos_2;
            blocked_2 &= -static_cast<uint64_t>((endpos_2 & blocks) == 0);
            dests |= endpos_3;
            blocked_3 &= -static_cast<uint64_t>((endpos_3 & blocks) == 0);
            dests |= endpos_4;
            blocked_4 &= -static_cast<uint64_t>((endpos_4 & blocks) == 0);
        }
        while(dests) {
            uint64_t to = 1ULL << __builtin_ctzll(dests);  
            dests &= dests - 1; 
            moves.emplace_back(fig,to);
        }
    }
    uint64_t shift_0  = SHIFTS[0][0];
    uint64_t shift_1  = SHIFTS[1][0];
    uint64_t not_figB = ~figuresB[0];
    uint64_t dests = 0;
    uint64_t endpos_1 = (guardB & MASK_LEFT_MOVES[0][0])  << shift_0 & not_figB;
    uint64_t endpos_2 = (guardB & MASK_LEFT_MOVES[1][0])  << shift_1 & not_figB;
    uint64_t endpos_3 = (guardB & MASK_RIGHT_MOVES[0][0]) >> shift_0 & not_figB;
    uint64_t endpos_4 = (guardB & MASK_RIGHT_MOVES[1][0]) >> shift_1 & not_figB;

    if (endpos_1) moves.emplace_back(guardB, endpos_1);
    if (endpos_2) moves.emplace_back(guardB, endpos_1);
    if (endpos_3) moves.emplace_back(guardB, endpos_1);
    if (endpos_4) moves.emplace_back(guardB, endpos_1);

    return moves;
}

