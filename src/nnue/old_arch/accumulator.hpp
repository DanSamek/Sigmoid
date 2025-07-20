#ifndef SIGMOID_OLD_ACCUMULATOR_HPP
#define SIGMOID_OLD_ACCUMULATOR_HPP

#include <array>
#include <cstdint>
#include <algorithm>

#include "nnue_consts.hpp"
#include "../../color.hpp"

namespace Sigmoid{
    struct OldAccumulator{
        std::array<int16_t, OLD_HIDDEN_LAYER_SIZE> data;

        OldAccumulator() = default;

        void add(const std::array<int16_t, OLD_HIDDEN_LAYER_SIZE>& weights) {
            for (int i = 0; i < OLD_HIDDEN_LAYER_SIZE; i++)
                data[i] += weights[i];
        }

        void sub(const std::array<int16_t, OLD_HIDDEN_LAYER_SIZE>& weights) {
            for (int i = 0; i < OLD_HIDDEN_LAYER_SIZE; i++)
                data[i] -= weights[i];
        }

        std::array<int16_t, OLD_HIDDEN_LAYER_SIZE>& get(){
            return data;
        }

        void init(std::array<int16_t, OLD_HIDDEN_LAYER_SIZE>& hiddenLayerBiases){
            data = hiddenLayerBiases;
        }
    };
}

#endif //SIGMOID_OLD_ACCUMULATOR_HPP
