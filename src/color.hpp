#ifndef SIGMOID_COLOR_HPP
#define SIGMOID_COLOR_HPP

#include "constants.hpp"

namespace Sigmoid{
    struct Color{
        int data = WHITE;

        operator int() const{
            return data;
        }

        Color flip() const{
            Color c;
            c.data = data == WHITE ? BLACK : WHITE;
            return c;
        }

        bool operator == (Color diff){
            return diff.data == data;
        }

        Color operator ~() const{
            return this->flip();
        }

        constexpr inline static Color white(){
            return Color(WHITE);
        }

        constexpr inline static Color black(){
            return Color(BLACK);
        }
    };

    template<Color us>
    inline constexpr Color opp(){
        return us.data == WHITE ? Color::black() : Color::white();
    }
}

#endif //SIGMOID_COLOR_HPP
