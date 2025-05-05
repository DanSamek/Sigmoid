#ifndef SIGMOID_BITOPS_HPP
#define SIGMOID_BITOPS_HPP

#include <cstdint>
#include <array>
#include <bit>

namespace Sigmoid {
    // Same implementation as in prev engine [Sentinel]
    static inline bool get_nth_bit(const uint64_t &value, int pos) {
        return value & (1ULL << pos);
    }

    static inline void set_nth_bit(uint64_t &value, int pos) {
        value |= (1ULL << pos);
    }

    static inline void pop_nth_bit(uint64_t &value, int pos) {
        if (get_nth_bit(value, pos)) value ^= (1ULL << pos);
    }

    static inline int bit_scan_forward(const uint64_t &value) {
        return std::countr_zero(value);
    }

    static inline int bit_scan_forward_pop_lsb(uint64_t &value) {
        auto result = bit_scan_forward(value);
        value &= value - 1;
        return result;
    }

    static inline int count_bits(uint64_t &bb) {
        int cnt = 0;
        while (bb) {
            cnt++;
            bit_scan_forward_pop_lsb(bb);
        }
        return cnt;
    }


}


#endif //SIGMOID_BITOPS_HPP
