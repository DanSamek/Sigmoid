#ifndef SIGMOID_HISTORY_HPP
#define SIGMOID_HISTORY_HPP

#include <array>
#include <cstdint>
#include <algorithm>

#include "constants.hpp"

namespace Sigmoid{
    template<int MaxValue, std::size_t... Dimensions>
    struct History;

    template<int MaxValue, std::size_t First, std::size_t... Rest>
    struct History<MaxValue, First, Rest...> {
        using type = std::array<typename History<MaxValue, Rest...>::type, First>;
        static constexpr int maxValue = MaxValue;
    };

    template<int MaxValue, std::size_t Last>
    struct History<MaxValue, Last> {
        using type = std::array<int, Last>;
        static constexpr int maxValue = MaxValue;
    };

    inline void apply_gravity(int& value, const int bonus, const int limit){
        int clampedBonus = std::clamp(bonus, -limit, limit);
        value += clampedBonus - value * abs(clampedBonus) / limit;
    }

    using KillerMoves = std::array<std::array<Move, 2>, MAX_PLY_P1>;
    const int KILLER_BONUS_0 = 300'000;
    const int KILLER_BONUS_1 = 200'000;

    const int MAX_CAP_HIST_BONUS = 30'000;
    // [from_pc][to_sq] [cap_pc]
    using CaptureHistory = History<MAX_CAP_HIST_BONUS, NUM_PIECES, NUM_SQUARES, NUM_PIECES>;

    using MainHistory = History<std::numeric_limits<int16_t>::max(), NUM_COLORS, NUM_SQUARES, NUM_SQUARES>;
    const int CONT_HIST_MAX_PLY = 1;
    const int MAX_CONT_HIST_BONUS = 20'000;
    // [prev_pc][prev_to_sq] [pc][to_sq]
    using ContinuationHistoryEntry = History<MAX_CONT_HIST_BONUS, NUM_PIECES, NUM_SQUARES, NUM_PIECES, NUM_SQUARES>;
    using ContinuationHistory = std::array<ContinuationHistoryEntry::type, CONT_HIST_MAX_PLY>;

    const int TT_MOVE_VALUE = 1'000'000;
    const int QUIET_OFFSET = KILLER_BONUS_0 + MAX_CAP_HIST_BONUS + 1; // max. quiet + -MAX_CAP_HIST bonus

    // Correction histories
    const int CORRECTION_HISTORY_ENTRIES = 32'768;
    const int MAX_CORRECTION_HISTORY_BONUS = 1'024;
    using PawnCorrectionHistory = std::array<std::array<int, CORRECTION_HISTORY_ENTRIES>, NUM_COLORS>;

    template<int Modulo>
    static inline uint64_t key(uint64_t value){
        return value % Modulo;
    }

    static inline uint64_t pawn_key(uint64_t value){
        return key<CORRECTION_HISTORY_ENTRIES>(value);
    }

}

#endif //SIGMOID_HISTORY_HPP
