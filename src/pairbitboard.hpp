#ifndef SIGMOID_PAIRBITBOARD_HPP
#define SIGMOID_PAIRBITBOARD_HPP

#include <cstdint>
#include <iostream>

#include "color.hpp"
#include "bitops.hpp"

namespace Sigmoid{
    // Wrapper for white and black piece bitboard - piece type.
    struct PairBitboard{
        uint64_t bitboards[2];

        template<Color color>
        uint64_t get() {
            return bitboards[color];
        }

        template<Color color>
        void set_bit(int square){
            set_nth_bit(bitboards[color], square);
        }

        template<Color color>
        void pop_bit(int square){
            pop_nth_bit(bitboards[color], square);
        }
        
        void reset(){
            bitboards[0] = bitboards[1] = 0ULL;
        }
    };
}

#endif //SIGMOID_PAIRBITBOARD_HPP
