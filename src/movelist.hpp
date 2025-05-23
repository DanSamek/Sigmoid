#ifndef SIGMOID_MOVELIST_HPP
#define SIGMOID_MOVELIST_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"

namespace Sigmoid {

    // TODO sort + score [?]
    struct MoveList {

        void add(const Move& move){
            moves[size++] = move;
        }

        Move get(){
            return iter_index < size ? moves[iter_index++] : Move::none();
        }

    private:
        std::array<Move, MAX_POSSIBLE_MOVES> moves;
        int size = 0;
        int iter_index = 0;

    };
}

#endif //SIGMOID_MOVELIST_HPP
