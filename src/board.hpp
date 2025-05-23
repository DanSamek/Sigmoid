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
#include "movegen.hpp"

namespace Sigmoid {
    struct Board{
        std::array<State, STACK_SIZE_P1> stateStack;
        int ply = 0;
        Color whoPlay;
        State currentState;

        Board(): ply(0) { }

        bool is_capture(const Move& move){
            return currentState.mailBox[move.to()] != NONE;
        }

        Piece at(int square){
            return currentState.mailBox[square];
        }

        bool make_move(const Move& move){
            return whoPlay == Color::white() ? make_move<Color::white()>(move) : make_move<Color::black()>(move);
        }

        // TODO nnue.push().
        // TODO update zobrist hash
        // TODO NNUE updates.
        template<Color us>
        bool make_move(const Move& move) {
            if (is_illegal<us>(currentState, move)) return false;

            State new_state = currentState;
            const bool is_cap = is_capture(move);
            const Piece piece = at(move.from());
            const int from = move.from();
            const int to = move.to();
            const Piece promo_piece = move.promo_piece();
            const Move::SpecialType special_type = move.special_type();

            Piece to_piece = piece;

            if (is_cap)
            {
                Piece captured = new_state.mailBox[to];
                assert(captured != NONE);
                new_state.bitboards[captured].pop_bit<opp<us>()>(to);
            }
            else if (promo_piece != NONE)
                to_piece = promo_piece;

            else if (special_type == Move::EN_PASSANT)
                handle_ep<us>(new_state, to);
            else if (special_type == Move::CASTLE)
                handle_castling<us>(new_state, from, to);
            else if (piece == Piece::PAWN && abs(from - to) > 8)
                new_state.enPassantSquare = us == WHITE ? from - 8 : from + 8;

            new_state.bitboards[piece].pop_bit<us>(from);
            new_state.bitboards[to_piece].set_bit<us>(to);

            new_state.mailBox[from] = NONE;
            new_state.mailBox[to] = to_piece;

            disable_castling<us>(new_state, piece, move);

            uint64_t tmp = new_state.bitboards[KING].get<us>();
            int new_king_pos = bit_scan_forward_pop_lsb(tmp);
            if (Movegen::is_square_attacked<us>(new_state, new_king_pos))
                return false;

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

                currentState.mailBox[square] = piece;
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
        void disable_castling(State& state, const Piece piece, const Move& move){
            if (!state.is_some_castling_set<us>())
                return;

            if (piece == ROOK)
                state.disable_castling_rook_move<us>(move.to());

            if (piece == KING)
                state.disable_castling<us>();
        }

        template<Color us>
        void handle_castling(State& state, int from, int to){
            const bool king_side = from > to;
            if (king_side){
                state.mailBox[to + 1] = NONE;
                state.mailBox[to - 1] = ROOK;
            }
            else{
                state.mailBox[to - 1] = NONE;
                state.mailBox[to + 1] = ROOK;
            }
            state.disable_castling<us>();
        }

        template<Color us>
        void handle_ep(State& state, int to){
            const int enemy_pawn_square = us == WHITE ? to + 8 : to - 8;
            state.bitboards[PAWN].pop_bit<opp<us>()>(enemy_pawn_square);
            state.mailBox[enemy_pawn_square] = NONE;
        }

        // Only for debug.
        void print_state() {
            for (int row = 0; row < 8; row++){
                for (int col = 0; col < 8; col++){
                    int i = get_square(row, col);
                    Piece piece = currentState.mailBox[i];

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

        // TODO - simple check for some types of the moves.
        //      - For example pins -- can be detected with ray
        //      - King moves to enemy attacks -> illegal - DONE
        //      - King castling -> are squares attacked [?] - DONE
        template<Color us>
        bool is_illegal(const State& state, const Move& move){
            if (move.special_type() == Move::CASTLE){
                const bool kingSide = move.from() > move.to();
                bool result = Movegen::is_square_attacked<us>(state, move.from());
                result |= Movegen::is_square_attacked<us>(state, kingSide ? move.from() + 1 : move.from() - 1);
                result |= Movegen::is_square_attacked<us>(state, move.to());
                return result;
            }
            else if (state.mailBox[move.from()] == KING)
                return Movegen::is_square_attacked<us>(state, move.to());

            // TODO pins.
            return false;
        }

        // TODO -- will be used in datagen -- oneday.
        std::string get_fen(){}
    };

}

#endif //SIGMOID_BOARD_HPP
