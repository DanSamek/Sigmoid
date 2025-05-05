#ifndef SIGMOID_COLOR_HPP
#define SIGMOID_COLOR_HPP

namespace Sigmoid{
    struct Color{
        bool white = true;

        Color flip(){
            Color c
            {
                white = !white
            };
            return c;
        }

        bool operator == (Color diff){
            return diff.white == white;
        }
    };
}

#endif //SIGMOID_COLOR_HPP
