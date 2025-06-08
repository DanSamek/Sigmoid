#ifndef SIGMOID_HISTORY_HPP
#define SIGMOID_HISTORY_HPP

#include <array>
#include <cstdint>
#include <algorithm>

#include "constants.hpp"

namespace Sigmoid{
    template<typename T, T MaxValue, std::size_t... Dimensions>
    struct History;

    template<typename T, T MaxValue, std::size_t First, std::size_t... Rest>
    struct History<T, MaxValue, First, Rest...> {
        using type = std::array<typename History<T, MaxValue, Rest...>::type, First>;

        static constexpr T maxValue = MaxValue;
    };

    template<typename T, T MaxValue, std::size_t Last>
    struct History<T, MaxValue, Last> {
        using type = std::array<T, Last>;

        static constexpr T maxValue = MaxValue;
    };

    template<typename T>
    inline void apply_gravity(T& value, const T bonus, const T limit){
        T clampedBonus = std::clamp(bonus, static_cast<T>(-limit), static_cast<T>(limit));
        value += clampedBonus - value * abs(clampedBonus) / limit;
    }

    using MainHistory = History<int16_t, std::numeric_limits<int16_t>::max(), NUM_COLORS, NUM_SQUARES, NUM_SQUARES>;

    const int CONT_HIST_MAX_PLY = 1;
    const int MAX_CONT_HIST_BONUS = 15000;
    // prev.from(), prev.to(), curr.from(), curr.to().
    using ContinuationHistoryEntry = History<int16_t, MAX_CONT_HIST_BONUS, NUM_SQUARES, NUM_SQUARES, NUM_SQUARES, NUM_SQUARES>;
    using ContinuationHistory = std::array<ContinuationHistoryEntry::type, CONT_HIST_MAX_PLY>;

    const int TT_MOVE_VALUE = 1000000;
    const int QUIET_OFFSET = MainHistory::maxValue + ContinuationHistoryEntry::maxValue * CONT_HIST_MAX_PLY;
}

#endif //SIGMOID_HISTORY_HPP
