#ifndef SIGMOID_COLOR_HPP
#define SIGMOID_COLOR_HPP

#include "constants.hpp"

namespace Sigmoid{
    enum Color{ WHITE, BLACK };

    template<Color us>
    inline constexpr Color opp(){
        return us == WHITE ? BLACK : WHITE;
    }

    constexpr Color operator ~(Color c) {
        return c == WHITE ? BLACK : WHITE;
    }
}

#endif //SIGMOID_COLOR_HPP
