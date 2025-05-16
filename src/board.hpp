#ifndef SIGMOID_BOARD_HPP
#define SIGMOID_BOARD_HPP

#include <cstdint>
#include <array>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>

#include "piece.hpp"
#include "state.hpp"
#include "constants.hpp"
#include "color.hpp"
#include "helper.hpp"

namespace Sigmoid {
    struct Board{

        std::array<State, STACK_SIZE_P1> stateStack;
        int ply = 0;
        Color whoPlay;
        State currentState;
        std::array<Piece, 64> mailBox;

        Board(): ply(0) {
            for(Piece& i : mailBox) i = NONE;
        }

        bool is_capture(const Move& move){
            return mailBox[move.to()] != NONE;
        }

        Piece at(int square){
            return mailBox[square];
        }


        bool make_move(const Move& move){
            return whoPlay == Color::white() ? make_move<Color::white()>(move) : make_move<Color::black()>(move);
        }

        // TODO nnue.push().
        // TODO update zobrist hash
        // TODO NNUE updates.

        template<Color us>
        bool make_move(const Move& move) {
            State new_state = currentState;
            if (is_illegal(new_state, move)) return false;

            const bool is_cap = is_capture(move);
            const Piece piece = at(move.from());
            const int from = move.from();
            const int to = move.from();
            const Piece promo_piece = move.promo_piece();
            const Move::SpecialType specialType = move.special_type();

            Piece to_piece = piece;

            if (is_cap)
            {
                Piece captured = mailBox[to];
                assert(captured != NONE);
                new_state.bitboards[captured].pop_bit<opp<us>()>(to);
            }
            else if (promo_piece != NONE)
                to_piece = promo_piece;

            else if (specialType == Move::EN_PASSANT)
                handle_ep<us>(new_state, from, to);
            else if (specialType == Move::CASTLE)
                handle_castling<us>(new_state, from, to);
            else if (piece == Piece::PAWN && abs(from - to) > 8)
                new_state.enPassantSquare = us == WHITE ? from - 8 : from + 8;

            new_state.bitboards[piece].pop_bit<us>(from);
            new_state.bitboards[to_piece].set_bit<us>(to);

            mailBox[from] = NONE;
            mailBox[to] = to_piece;

            whoPlay = whoPlay.flip();
            stateStack[ply] = currentState;
            currentState = new_state;
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

                mailBox[square] = piece;
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


        template<Color us>
        void handle_castling(State& state, int from, int to){

        }

        template<Color us>
        void handle_ep(State& state, int from, int to){

        }

        // Only for debug.
        void print_state() {
            for (int row = 0; row < 8; row++){
                for (int col = 0; col < 8; col++){
                    int i = get_square(row, col);
                    Piece piece = mailBox[i];

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

        // TODO -- simple check of a move, that is on 100% illegal and we can return from make_move false.
        // For example pins -- can be detected with rays.
        bool is_illegal(const State& state, const Move& move){
            return false;
        }

        // TODO -- will be used in datagen -- oneday.
        std::string get_fen(){}
    };

}

#endif //SIGMOID_BOARD_HPP
