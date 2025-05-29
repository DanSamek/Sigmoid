#ifndef SIGMOID_MOVELIST_HPP
#define SIGMOID_MOVELIST_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"
#include "movegen.hpp"

namespace Sigmoid {

    struct MoveList {

        void add(const Move& move){
            moves[size++] = move;
        }

        Move get(){
            return iterIndex < size ? moves[iterIndex++] : Move::none();
        }

    private:
        std::array<Move, MAX_POSSIBLE_MOVES> moves;
        int size = 0;
        int iterIndex = 0;

    };
}

#endif //SIGMOID_MOVELIST_HPP
