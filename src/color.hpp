#ifndef SIGMOID_COLOR_HPP
#define SIGMOID_COLOR_HPP

#include "constants.hpp"

namespace Sigmoid{
    struct Color{
        static inline constexpr int WHITE = 0;
        static inline constexpr int BLACK = 1;
        int data = WHITE;

        operator int() const{
            return data;
        }

        Color flip(){
            Color c
            {
                data = data == WHITE ? BLACK : WHITE
            };
            return c;
        }

        bool operator == (Color diff){
            return diff.data == data;
        }

        constexpr inline static Color white(){
            return Color(WHITE);
        }

        constexpr inline static Color black(){
            return Color(BLACK);
        }
    };
}

#endif //SIGMOID_COLOR_HPP
