#ifndef SIGMOID_CONSTANTS_HPP
#define SIGMOID_CONSTANTS_HPP

namespace Sigmoid {
    constexpr int MAX_PLY = 128;
    constexpr int MAX_PLY_P1 = MAX_PLY + 1;
    constexpr int MAX_POSSIBLE_MOVES = 218;

    constexpr int STACK_SIZE = 2048;
    constexpr int STACK_SIZE_P1 = STACK_SIZE + 1;

    constexpr int NO_EN_PASSANT_SQUARE = -1;

    constexpr uint8_t NO_SQUARE = -1;

    static inline constexpr int WHITE = 0;
    static inline constexpr int BLACK = 1;
}
#endif //SIGMOID_CONSTANTS_HPP
