#ifndef SIGMOID_CONSTANTS_HPP
#define SIGMOID_CONSTANTS_HPP

namespace Sigmoid {
    const std::string VERSION = "0.0.1";

    const int MAX_PLY = 128;
    const int MAX_PLY_P1 = MAX_PLY + 1;
    constexpr int MAX_POSSIBLE_MOVES = 218;

    constexpr int STACK_SIZE = 2048;
    constexpr int STACK_SIZE_P1 = STACK_SIZE + 1;

    constexpr uint8_t NO_SQUARE = -1;

    constexpr int16_t MAX_VALUE = 32000;
    constexpr int16_t MIN_VALUE = -32000;
    constexpr int16_t CHECKMATE = MAX_VALUE - (MAX_PLY + 1);
    constexpr int16_t CHECKMATE_BOUND = 31000;
    constexpr int16_t DRAW = 0;

    constexpr int16_t NUM_SQUARES = 64;
    constexpr int16_t NUM_PIECES  = 6;
    constexpr int16_t NUM_COLORS  = 2;

}
#endif //SIGMOID_CONSTANTS_HPP
