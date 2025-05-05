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
#include <iostream>

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
        std::array<State, STACK_SIZE_P1> stateStack;
        int ply = 0;
        Color whoPlay;
        State currentState;

        bool make_move() {
            // TODO nnue.push().

            // TODO make move
            // TODO update mailbox.
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

            // TODO NNUE nnue.pop()
            assert(ply >= 1);
            ply--;
        }

        // TODO
        void load_from_fen(std::string fen){
            currentState.reset();

            int row = 0;
            for (char c: fen){
                if (c == '/')
                    row ++;

                if (c == ' ')
                    break;
            }
        }

        // TODO
        std::string get_fen(){

        }

        void print_board() {
            for (int i = 0; i < 64; i++){
                Piece piece = currentState.mailBox.at(i);
                currentState.bitboards[PAWN].get<Color::white()>();
                char c = piece_char<Color::white()>(piece);

                std::cout << c << std::endl;
            }
        }

    };

}

#endif //SIGMOID_BOARD_HPP
