#ifndef SIGMOID_MOVELIST_HPP
#define SIGMOID_MOVELIST_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"
#include "movegen.hpp"

namespace Sigmoid {

    template<bool captures>
    struct MoveList {
        MoveList(const Board* board) : board(board){}

        Move get(){
            if (!generated){
                Movegen::generate_moves<captures>(board->currentState, board->whoPlay, moves, size);
                generated = true;
            }
            return iterIndex < size ? moves[iterIndex++] : Move::none();
        }

    private:
        const Board* board;
        std::array<Move, MAX_POSSIBLE_MOVES> moves;
        int size = 0;


        bool generated = false;
        int iterIndex = 0;

    };
}

#endif //SIGMOID_MOVELIST_HPP
