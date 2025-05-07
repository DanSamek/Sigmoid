#ifndef SIGMOID_MOVELIST_HPP
#define SIGMOID_MOVELIST_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"

namespace Sigmoid{

    struct MoveList {
        void add(const Move& move){
            _moves[index++] = move;
        }

        // TODO iterators + sort + score [?]
    private:
        std::array<Move, MAX_POSSIBLE_MOVES> _moves;
        int index = 0;

    };
}

#endif //SIGMOID_MOVELIST_HPP
