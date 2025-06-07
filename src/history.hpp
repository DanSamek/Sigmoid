#ifndef SIGMOID_HISTORY_HPP
#define SIGMOID_HISTORY_HPP

#include <array>
#include <cstdint>
#include <algorithm>

#include "constants.hpp"

namespace Sigmoid{
    template<typename T, std::size_t... Dimensions>
    struct History;

    template<typename T, std::size_t First, std::size_t... Rest>
    struct History<T, First, Rest...> {
        using type = std::array<typename History<T, Rest...>::type, First>;
    };

    template<typename T, std::size_t Last>
    struct History<T, Last> {
        using type = std::array<T, Last>;
    };

    template<typename T>
    inline void apply_gravity(T& value, const T bonus){
        constexpr T limit = std::numeric_limits<T>::max();
        T clampedBonus = std::clamp(bonus, static_cast<T>(-limit), static_cast<T>(limit));
        value += clampedBonus - value * abs(clampedBonus) / limit;
    }

    using MainHistory = History<int16_t, NUM_COLORS, NUM_SQUARES, NUM_SQUARES>::type;

    const int TT_MOVE_VALUE = 1000000;
    const int QUIET_OFFSET = std::numeric_limits<int16_t>::max();
}

#endif //SIGMOID_HISTORY_HPP
