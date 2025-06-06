#ifndef SIGMOID_TIMER_HPP
#define SIGMOID_TIMER_HPP

#include <cstdint>
#include <chrono>
#include <algorithm>

#include "color.hpp"

namespace Sigmoid {
    struct Timer{
        int64_t toSearch = 0;
        std::chrono::high_resolution_clock::time_point startTime;

        Timer(int64_t wTime, int64_t bTime, int64_t wInc, int64_t bInc, Color us){
            toSearch = us == WHITE ? getSearchTime(wTime, wInc) : getSearchTime(bTime, bInc);
            startTime = std::chrono::high_resolution_clock::now();
        }

        [[nodiscard]] bool is_time_out() const{
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = now - startTime;
            return (elapsed.count() * 1000 ) >= toSearch;
        }

        [[nodiscard]] int64_t get_ms() const{
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
            return elapsed.count();
        }

        [[nodiscard]] static int64_t getSearchTime(int64_t timeRemaining, int64_t increment) {
            constexpr int64_t minMs = 5;

            int64_t msCanBeUsed = timeRemaining / 25;
            msCanBeUsed += increment / 2 + increment / 4;

            if(msCanBeUsed >= timeRemaining)
                msCanBeUsed = std::clamp(msCanBeUsed, minMs, timeRemaining / 25);

            return msCanBeUsed;
        }
    };
}

#endif //SIGMOID_TIMER_HPP
