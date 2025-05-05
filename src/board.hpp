#ifndef SIGMOID_BOARD_HPP
#define SIGMOID_BOARD_HPP

#include <cstdint>
#include <array>
#include <cassert>
#include <string>

#include "piece.hpp"
#include "state.hpp"
#include "constants.hpp"
#include "color.hpp"

namespace Sigmoid {
    /* Board representation.
        0 1 2 3 4 5 6 7
      0
      1
      2
      3
      4
      5
      6
      7
    */
    struct Board{
        std::array<State, MAX_PLY_P1> stateStack;
        int ply = 0;
        Color whoPlay;
        State currentState;

        bool make_move() {
            // TODO nnue.push().

            // TODO make move
            // TODO update zobrist hash
            // TODO NNUE updates.

            ply++;
            stateStack[ply] = currentState;
            return true;
        }

        void undo_move(){
            // Stack pop
            currentState = stateStack[ply];
            whoPlay = whoPlay.flip();

            // NNUE pop TODO

            assert(ply >= 1);
            ply--;
        }

        // TODO
        void load_from_fen(std::string fen){

        }

        // TODO
        std::string get_fen(){

        }
    };

}


#endif //SIGMOID_BOARD_HPP
