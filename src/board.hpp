#ifndef SIGMOID_BOARD_HPP
#define SIGMOID_BOARD_HPP

#include <cstdint>
#include <array>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>

#include "piece.hpp"
#include "state.hpp"
#include "constants.hpp"
#include "color.hpp"
#include "helper.hpp"

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

            stateStack[ply] = currentState;
            ply++;
            return true;
        }

        void undo_move(){
            assert(ply >= 1);
            ply--;

            // Stack pop
            currentState = stateStack[ply];
            whoPlay = whoPlay.flip();

            // TODO NNUE nnue.pop()
         }

        void load_from_fen(std::string fen){
            currentState.reset();

            size_t square = 0;
            size_t i = 0;
            for (i = 0; i < fen.length(); i++){
                char c = fen[i];
                if (c == '/')
                    continue;

                int c_int = c - '0';
                if (c_int >= 1 && c_int <= 8){
                    square += c_int;
                    continue;
                }

                if (c == ' ')
                    break;

                Piece piece = LETTER_PIECE_MAP.at(tolower(c));

                if (isupper(c))
                    currentState.set_bit<Color::white()>(square, piece);
                else
                    currentState.set_bit<Color::black()>(square, piece);

                currentState.mailBox.set_piece(square, piece);
                square++;
            }
            ++i; // move to a color index in fen.
            assert(i < fen.length());
            whoPlay = fen[i] == 'w' ? Color::white() : Color::black();

            // Castling
            i += 2;
            if (fen[i] != '-'){
                for (; i < fen.length(); i++){
                    char c = fen[i];
                    if (c == ' ')
                        break;

                    currentState.set_casting_index(c);
                }
                i++;
            }

            // En-passant
            if (fen[i] != '-'){
                int col = (fen[i] - 'a');
                int row = 7 - (fen[++i] - '1');
                currentState.enPassantSquare = get_square(row, col);
            }

            i++;
            std::string substr = fen.substr(i);
            std::stringstream ss(substr);
            ss >> currentState.halfMove >> currentState.fullMove;
        }

        // TODO -- will be used in datagen -- oneday.
        std::string get_fen(){}

        // Only for debug.
        void print_state() {
            for (int row = 0; row < 8; row++){
                for (int col = 0; col < 8; col++){
                    int i = get_square(row, col);
                    Piece piece = currentState.mailBox.at(i);

                    if (i == currentState.enPassantSquare){
                        std::cout << "E";
                        continue;
                    }
                    if (piece == NONE){
                        std::cout << "-";
                        continue;
                    }
                    bool upper = currentState.get_bit<Color::white()>(i, piece);
                    char c = upper ? piece_char<Color::white()>(piece) : piece_char<Color::black()>(piece);
                    std::cout << c;
                }
                std::cout << std::endl;
            }

            std::cout << "HalfMove: " << currentState.halfMove << std::endl;
            std::cout << "FullMove: " << currentState.fullMove << std::endl;
            std::cout << "Castling: " << (int)currentState.castling << std::endl;
            std::cout << std::endl;
        }
    };

}

#endif //SIGMOID_BOARD_HPP
