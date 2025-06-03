#ifndef SIGMOID_ACCUMULATOR_HPP
#define SIGMOID_ACCUMULATOR_HPP

#include <array>
#include <cstdint>
#include <algorithm>

#include "nnue_consts.hpp"
#include "../color.hpp"

namespace Sigmoid{
    struct Accumulator{
        std::array<std::array<int16_t, HIDDEN_LAYER_SIZE>, 2> data;

        Accumulator(){}

        template<Color color>
        void add(const std::array<int16_t, HIDDEN_LAYER_SIZE>& weights) {
            for (int i = 0; i < HIDDEN_LAYER_SIZE; i++)
                data[color][i] += weights[i];
        }

        template<Color color>
        void sub(const std::array<int16_t, HIDDEN_LAYER_SIZE>& weights) {
            for (int i = 0; i < HIDDEN_LAYER_SIZE; i++)
                data[color][i] -= weights[i];
        }

        template<Color color>
        std::array<int16_t, HIDDEN_LAYER_SIZE>& get(){
            return data[color];
        }

        void init(std::array<int16_t, HIDDEN_LAYER_SIZE>& hiddenLayerBiases){
            data[WHITE] = hiddenLayerBiases;
            data[BLACK] = hiddenLayerBiases;
        }
    };
}

#endif //SIGMOID_ACCUMULATOR_HPP
