#ifndef SIGMOID_MOVEGEN_HPP
#define SIGMOID_MOVEGEN_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"

namespace Sigmoid{
    struct Movegen{

        template<bool captures>
        void generate_moves(std::array<Move, MAX_POSSIBLE_MOVES>& moves){
            // TODO
        }
    };
}

#endif //SIGMOID_MOVEGEN_HPP
