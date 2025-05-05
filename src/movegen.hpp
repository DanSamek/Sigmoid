#ifndef SIGMOID_MOVEGEN_HPP
#define SIGMOID_MOVEGEN_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"
#include "board.hpp"
#include "magics.hpp"

namespace Sigmoid{
    // Pseudo-legal movegen.
    struct Movegen{

        template<bool captures>
        void generate_moves(const Board& board, std::array<Move, MAX_POSSIBLE_MOVES>& result_moves){
            // TODO
        }
    };
}

#endif //SIGMOID_MOVEGEN_HPP
