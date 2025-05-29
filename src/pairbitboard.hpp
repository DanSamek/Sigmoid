#ifndef SIGMOID_PAIRBITBOARD_HPP
#define SIGMOID_PAIRBITBOARD_HPP

#include <cassert>
#include <cstdint>
#include <iostream>

#include "color.hpp"
#include "bitops.hpp"

namespace Sigmoid{
    // Wrapper for white and black piece bitboard - piece type.
    struct PairBitboard{
        std::array<uint64_t, 2> bitboards;

        PairBitboard(){
            clear();
        }

        template<Color color>
        inline uint64_t get() const{
            return bitboards[color];
        }

        template<Color color>
        inline bool get(int square) const{
            assert(square >= 0 && square <= 63);
            bool result = get_nth_bit(bitboards[color], square);
            return result;
        }

        template<Color color>
        inline void set_bit(int square){
            assert(square >= 0 && square <= 63);
            set_nth_bit(bitboards[color], square);
        }

        template<Color color>
        inline void pop_bit(int square){
            assert(square >= 0 && square <= 63);
            pop_nth_bit(bitboards[color], square);
        }
        
        void clear(){
            bitboards[0] = bitboards[1] = 0ULL;
        }
    };
}

#endif //SIGMOID_PAIRBITBOARD_HPP
