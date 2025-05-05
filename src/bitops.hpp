#ifndef SIGMOID_BITOPS_HPP
#define SIGMOID_BITOPS_HPP

#include <cstdint>
#include <array>
#include <bit>

namespace Sigmoid {
    // Same implementation as in prev engine [Sentinel]
    template<typename T>
    static inline bool get_nth_bit(const T &value, int pos) {
        return value & (1ULL << pos);
    }

    template<typename T>
    static inline void set_nth_bit(T &value, int pos) {
        value |= (1ULL << pos);
    }

    template<typename T>
    static inline void pop_nth_bit(T &value, int pos) {
        if (get_nth_bit(value, pos)) value ^= (1ULL << pos);
    }

    template<typename T>
    static inline int bit_scan_forward(const T &value) {
        return std::countr_zero(value);
    }

    template<typename T>
    static inline int bit_scan_forward_pop_lsb(T &value) {
        auto result = bit_scan_forward(value);
        value &= value - 1;
        return result;
    }

    template<typename T>
    static inline int count_bits(T &bb) {
        int cnt = 0;
        while (bb) {
            cnt++;
            bit_scan_forward_pop_lsb(bb);
        }
        return cnt;
    }


}


#endif //SIGMOID_BITOPS_HPP
