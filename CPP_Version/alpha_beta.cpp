#include "calculations.hpp"
#include "const.hpp"
#include "board.hpp"
#include <array>

using Move = std::array<uint64_t, 2>;

Move alpha_beta(MoveHistory &stack, uint64_t from, uint64_t to, uint64_t (&figuresB)[7], uint64_t (&figuresR)[7], uint8_t (&figuresB_2d)[49], uint8_t (&figuresR_2d)[49], uint64_t &guardB, uint64_t &guardR, bool isBlueTurn) {
    
}
    