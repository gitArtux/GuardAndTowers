
#include "alpha_beta.hpp"

using namespace masks;

ScoredMove beta_search(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int depth, int alpha, int beta);
ScoredMove alpha_search(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int depth, int alpha, int beta);


// // Eval without useage of enemy use hits
int evaluate(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint64_t guardB, uint64_t guardR, bool isBlueTurn) {
    uint64_t fb0 = figuresB[0];
    uint64_t fb1 = figuresB[1];
    uint64_t fb2 = figuresB[2];

    uint64_t fr0 = figuresR[0];
    uint64_t fr1 = figuresR[1];
    uint64_t fr2 = figuresR[2];

    if (!guardR || guardB & HOMESQUARE_R)  {
        return MAX_SCORE; 
    } else if (!guardB || guardR & HOMESQUARE_B) {
        return MIN_SCORE; 
    } else if  (isBlueTurn && (fb0 << 7 | fb0 << 1 | fb0 >>1 | fb0 >> 7) & guardR) {
        return MAX_SCORE; 
    } else if (!isBlueTurn && (fr0 >> 7 | fr0 << 1 | fr0 >>1 | fr0 << 7) & guardB) {
        return MIN_SCORE; 
    }


      
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

    
    score += 300 * figure_diff; 

    // Stack Height Values
    score -= 10 * (stack_height_B_0-stack_height_R_0); 
    score += 20 * (stack_height_B_1-stack_height_R_1);
    score += 10 * (stack_height_B_2-stack_height_R_2);
    score += 40 * (stack_height_B_3-stack_height_R_3);

    // higher stacks already penalized by having fewer lower stacks

  

    // positionanl
    // heigth 1
    
    

    score -= 10 * (__builtin_popcountll(fb0 & ROW_1) - __builtin_popcountll(fr0 & ROW_7)); // Row 1
    score += 10 * (__builtin_popcountll(fb0 & ROW_2) - __builtin_popcountll(fr0 & ROW_6)); // Row 2
    score += 10 * (__builtin_popcountll(fb0 & ROW_3) - __builtin_popcountll(fr0 & ROW_5)); // Row 3
    score += 10 * (__builtin_popcountll(fb0 & ROW_4) - __builtin_popcountll(fr0 & ROW_4)); // Row 4
    score += 20 * (__builtin_popcountll(fb0 & ROW_5) - __builtin_popcountll(fr0 & ROW_3)); // Row 5
    score += 20 * (__builtin_popcountll(fb0 & ROW_6) - __builtin_popcountll(fr0 & ROW_2)); // Row 6
    score += 20 * (__builtin_popcountll(fb0 & ROW_7) - __builtin_popcountll(fr0 & ROW_1)); // Row 7

    // heigth 2
    score += 10 * (__builtin_popcountll(fb1 & ROW_1) - __builtin_popcountll(fr1 & ROW_7)); // Row 1
    score += 10 * (__builtin_popcountll(fb1 & ROW_2) - __builtin_popcountll(fr1 & ROW_6)); // Row 2
    score += 40 * (__builtin_popcountll(fb1 & ROW_3) - __builtin_popcountll(fr1 & ROW_5)); // Row 3
    score += 30 * (__builtin_popcountll(fb1 & ROW_4) - __builtin_popcountll(fr1 & ROW_4)); // Row 4
    score += 50 * (__builtin_popcountll(fb1 & ROW_5) - __builtin_popcountll(fr1 & ROW_3)); // Row 5
    score += 30 * (__builtin_popcountll(fb1 & ROW_6) - __builtin_popcountll(fr1 & ROW_2)); // Row 6
    score += 30 * (__builtin_popcountll(fb1 & ROW_7) - __builtin_popcountll(fr1 & ROW_1)); // Row 7


    // height 2 & blocking
    // block at least 1s
    score += 10 * (__builtin_popcountll(((fb1 & ROW_4) << 7) & fr0) - __builtin_popcountll(((fr1 & ROW_4) >> 7) & fb0)); // Row 4
    score += 30 * (__builtin_popcountll(((fb1 & ROW_5) << 7) & fr0) - __builtin_popcountll(((fr1 & ROW_3) >> 7) & fb0)); // Row 5
    score += 30 * (__builtin_popcountll(((fb1 & ROW_6) << 7) & fr0) - __builtin_popcountll(((fr1 & ROW_2) >> 7) & fb0)); // Row 6

    // block a least 2s
    score += 10 * (__builtin_popcountll(((fb1 & ROW_4) << 7) & fr1) - __builtin_popcountll(((fr1 & ROW_4) >> 7) & fb1)); // Row 4
    score += 20 * (__builtin_popcountll(((fb1 & ROW_5) << 7) & fr1) - __builtin_popcountll(((fr1 & ROW_3) >> 7) & fb1)); // Row 5
    score += 20 * (__builtin_popcountll(((fb1 & ROW_6) << 7) & fr1) - __builtin_popcountll(((fr1 & ROW_2) >> 7) & fb1)); // Row 6

    // block 2s 2away
    score += 10 * (__builtin_popcountll(((fb2 & ROW_4) << 14) & fr1) - __builtin_popcountll(((fr2 & ROW_4) >> 14) & fb1)); // Row 4
    score += 20 * (__builtin_popcountll(((fb2 & ROW_5) << 14) & fr1) - __builtin_popcountll(((fr2 & ROW_3) >> 14) & fb1)); // Row 5

    // Guard Safety
    score += 80 * (__builtin_popcountll(((fb0 << 6) | (fb0 << 8)) & guardR) - __builtin_popcountll(((fr0 >> 6) | (fr0 >> 8)) & guardB)); // Row 1
    
    return score;
}


inline int max(int a, int b) {
    return (a > b) ? a : b;
}

inline int min(int a, int b) {
    return (a < b) ? a : b;
}




ScoredMove alpha_search(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int depth, int alpha, int beta) {
    // Check for terminal conditions
    ScoredMove bestmove;
    int max_eval = MIN_SCORE;
    // std::cout << "Before Sort" << std::endl; // Debugging output  
    // print_board(figuresB, figuresR, guardB, guardR, true); // Print the board before the move
    
    Moves moves = move_generation(figuresB, figuresR, figuresB_2d, guardB, guardR);
    ScoredMoves scored_moves;
    scored_moves.reserve(moves.size());
    for (auto &move : moves) {
        do_move(move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR);
        scored_moves.push_back({move, evaluate(figuresB, figuresR, guardB, guardR, false)});        
        undo(move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR);
        if (scored_moves.back().score == MAX_SCORE) return scored_moves.back();
    }
    // print_board(figuresB, figuresR, guardB, guardR, true); // Print the board before the move
    // Sort the moves based on their scores in descending order
    std::sort(scored_moves.begin(), scored_moves.end(), [](const ScoredMove& a, const ScoredMove& b) {return a.score > b.score;});

    // std::cout << "After Sort" << std::endl; // Debugging output
    // print_board(figuresB, figuresR, guardB, guardR, true); // Print the board before the move
    
    if (depth == 1) return scored_moves[0]; // If depth is 1, return the best move directly
    for (auto &scoredmove : scored_moves) {
        // Make the move
        // std::cout << "Before: " << FEN_Move(scoredmove.move) << std::endl; // Extract FEN moves for debugging
        // print_board(figuresB, figuresR, guardB, guardR, false); // Print the board after the move


        do_move(scoredmove.move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR);

        // std::cout << "Do" << std::endl; // Extract FEN moves for debugging
        // print_board(figuresB, figuresR, guardB, guardR, false); // Print the board after the move

        ScoredMove betamove = beta_search(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, depth - 1, alpha, beta);
        int eval = betamove.score;
        
        // Undo the move
        undo(scoredmove.move, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR);
        // std::cout << "Undo" << std::endl; // Extract FEN moves for debugging
        // print_board(figuresB, figuresR, guardB, guardR, true); // Print the board after the move


        // Check if the move is better than the current best move
        if (eval >= max_eval) {
            max_eval = eval;
            bestmove = scoredmove;
        }

        alpha = max(alpha, eval);
        if (alpha >= beta) {
            // Beta cut-off
            break;
        }
    }   
    return bestmove; 
}

ScoredMove beta_search(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, int depth, int alpha, int beta) {
    
    int min_eval = MAX_SCORE;
    ScoredMove bestmove;
    // std::cout << "Before Sort" << std::endl; // Debugging output
    // print_board(figuresB, figuresR, guardB, guardR, true); // Print the board before the move
    Moves moves = move_generation(figuresR, figuresB, figuresR_2d, guardR, guardB);
    ScoredMoves scored_moves;
    // std::cout << "FEN_Moves:" << extract_FEN_Moves(moves) << std::endl;
    // print_board(figuresB, figuresR, guardB, guardR, false); // Print the board before the move
    scored_moves.reserve(moves.size());
    for (auto &move : moves) {
        do_move(move, figuresR, figuresB, figuresR_2d, figuresB_2d, guardR, guardB);
        scored_moves.push_back({move, evaluate(figuresB, figuresR, guardB, guardR, true)});       
        undo(move, figuresR, figuresB, figuresR_2d, figuresB_2d, guardR, guardB);
        if (scored_moves.back().score == MIN_SCORE) {
            return scored_moves.back(); // If the move leads to a guaranteed win, return it immediately
        }
    }
    // Sort the moves based on their scores in ascending order
    std::sort(scored_moves.begin(), scored_moves.end(), [](const ScoredMove& a, const ScoredMove& b) {return a.score < b.score;});

    // std::cout << "After Sort" << std::endl; // Debugging output
    
    // print_board(figuresB, figuresR, guardB, guardR, true); // Print the board before the move
    
    if (depth == 1) return scored_moves[0]; // If depth is 1, return the best move directly
    for (auto &scoredmove : scored_moves) {
        // std::cout << "Before: " << FEN_Move(scoredmove.move) << std::endl; // Extract FEN moves for debugging
        // print_board(figuresB, figuresR, guardB, guardR, false); // Print the board after the move

        do_move(scoredmove.move, figuresR, figuresB, figuresR_2d, figuresB_2d, guardR, guardB);
        
        // std::cout << "Do/ before alphasearch" << std::endl; // Extract FEN moves for debugging
        // print_board(figuresB, figuresR, guardB, guardR, true); 
        ScoredMove alphamove = alpha_search(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, depth - 1, alpha, beta);
        int eval = alphamove.score;

        // std::cout << "Do/ after alphasearch" << std::endl; // Extract FEN moves for debugging
        // print_board(figuresB, figuresR, guardB, guardR, true);

        
        undo(scoredmove.move, figuresR, figuresB, figuresR_2d, figuresB_2d, guardR, guardB);

        // std::cout << "Undo" << std::endl; // Extract FEN moves for debugging
        // print_board(figuresB, figuresR, guardB, guardR, false);



        // Check if the move is better than the current best move
        if (eval <= min_eval) {
            min_eval = eval;
            bestmove = scoredmove;
        }
        beta = min(beta, eval);
        if (beta <= alpha) {
            break;
        }
    }   
    return bestmove;
}


Move alpha_beta(uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool turnB, int depth) {
    if (turnB) {
        return alpha_search(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, depth, -100000, 100000).move;
    }
    return beta_search(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, depth, -100000, 100000).move;
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


std::string make_move(std::string fen_pos, int depth){
    uint64_t figuresB[7];
    uint64_t figuresR[7];
    uint8_t figuresB_2d[49];
    uint8_t figuresR_2d[49];
    uint64_t guardB;
    uint64_t guardR;
    bool isBlueTurn;
    // int depth = 5;

    set_board(fen_pos, figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn);
    return res_FEN_Move(alpha_beta(figuresB, figuresR, figuresB_2d, figuresR_2d, guardB, guardR, isBlueTurn, depth));
}