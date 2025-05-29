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
            return currentState.pieceMap[move.to()] != NONE;
        }

        Piece at(int square){
            return currentState.pieceMap[square];
        }

        bool make_move(const Move& move){
            return whoPlay == WHITE ? make_move<WHITE>(move) : make_move<BLACK>(move);
        }

        // TODO nnue.push().
        // TODO update zobrist hash
        // TODO NNUE updates.
        template<Color us>
        bool make_move(const Move& move) {
            if (is_illegal<us>(currentState, move)) return false;

            constexpr Color op = ~us;
            State new_state = currentState;
            new_state.enPassantSquare = 0;

            const bool is_cap = is_capture(move);
            const Piece piece = at(move.from());
            const int from = move.from();
            const int to = move.to();
            const Piece promo_piece = move.promo_piece();
            const Move::SpecialType special_type = move.special_type();

            Piece to_piece = piece;

            if (is_cap)
            {
                Piece captured = new_state.pieceMap[to];
                assert(captured != NONE);
                disable_cap_castling<us>(new_state, move);
                new_state.bitboards[captured].pop_bit<op>(to);
            }
            else{
                switch (special_type) {
                    case Move::EN_PASSANT:
                        handle_ep<us>(new_state, to);
                        break;
                    case Move::CASTLE:
                        handle_castling<us>(new_state, from, to);

                    case Move::NONE:
                        if (piece == Piece::PAWN && abs(from - to) > 8)
                            new_state.enPassantSquare = us == WHITE ? from - 8 : from + 8;
                        break;
                    default:
                        break;
                }
            }
            if (promo_piece != NONE)
                to_piece = promo_piece;

            new_state.bitboards[piece].pop_bit<us>(from);
            new_state.bitboards[to_piece].set_bit<us>(to);

            new_state.pieceMap[from] = NONE;
            new_state.pieceMap[to] = to_piece;

            disable_castling<us>(new_state, piece, move);

            uint64_t tmp = new_state.bitboards[KING].get<us>();
            int new_king_pos = bit_scan_forward_pop_lsb(tmp);
            if (Movegen::is_square_attacked<us>(new_state, new_king_pos))
                return false;

            whoPlay = ~whoPlay;
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
            whoPlay = ~whoPlay;

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
                    currentState.set_bit<WHITE>(square, piece);
                else
                    currentState.set_bit<BLACK>(square, piece);

                currentState.pieceMap[square] = piece;
                square++;
            }
            ++i; // move to a color index in fen.
            assert(i < fen.length());
            whoPlay = fen[i] == 'w' ? WHITE : BLACK;

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
            else
                i += 2;

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
        void disable_cap_castling(State& state, const Move& move){
            if (!state.is_some_castling_set<opp<us>()>())
                return;

            uint64_t enemy_rook_bb_cpy = state.bitboards[ROOK].get<opp<us>()>();

            int to = move.to();
            if constexpr (us == BLACK){
                if (to == 63 && get_nth_bit(enemy_rook_bb_cpy, to))
                    state.disable_castling_index(State::K_CASTLING_BIT);

                if (to == 56 && get_nth_bit(enemy_rook_bb_cpy, to))
                    state.disable_castling_index(State::Q_CASTLING_BIT);
            }
            else{
                if (to == 7 && get_nth_bit(enemy_rook_bb_cpy, to))
                    state.disable_castling_index(State::k_CASTLING_BIT);

                if (to == 0 && get_nth_bit(enemy_rook_bb_cpy, to))
                    state.disable_castling_index(State::q_CASTLING_BIT);
            }
        }

        template<Color us>
        void move_piece(State& state, int from, int to, Piece piece){
            state.bitboards[piece].pop_bit<us>(from);
            state.bitboards[piece].set_bit<us>(to);
        }

        template<Color us>
        void disable_castling(State& state, const Piece piece, const Move& move){
            if (!state.is_some_castling_set<us>())
                return;

            if (piece == ROOK)
                state.disable_castling_rook_move<us>(move.from());

            if (piece == KING)
                state.disable_castling<us>();
        }

        template<Color us>
        void handle_castling(State& state, int from, int to){
            const bool king_side = from < to;
            if (king_side){
                state.pieceMap[to + 1] = NONE;
                state.pieceMap[to - 1] = ROOK;
                move_piece<us>(state, to + 1, to - 1, ROOK);
            }
            else{
                state.pieceMap[to - 2] = NONE;
                state.pieceMap[to + 1] = ROOK;
                move_piece<us>(state, to - 2, to + 1, ROOK);
            }
            state.disable_castling<us>();
        }

        template<Color us>
        void handle_ep(State& state, int to){
            const int enemy_pawn_square = us == WHITE ? to + 8 : to - 8;
            state.bitboards[PAWN].pop_bit<~us>(enemy_pawn_square);
            state.pieceMap[enemy_pawn_square] = NONE;
        }

        // Only for debug.
        void print_state() const{
            for (int row = 0; row < 8; row++){
                for (int col = 0; col < 8; col++){
                    int i = get_square(row, col);
                    Piece piece = currentState.pieceMap[i];

                    if (i == currentState.enPassantSquare){
                        std::cout << "E";
                        continue;
                    }
                    if (piece == NONE){
                        std::cout << "-";
                        continue;
                    }
                    bool upper = currentState.get_bit<WHITE>(i, piece);
                    char c = upper ? piece_char<WHITE>(piece) : piece_char<BLACK>(piece);
                    std::cout << c;
                }
                std::cout << std::endl;
            }

            std::cout << "HalfMove: " << currentState.halfMove << std::endl;
            std::cout << "FullMove: " << currentState.fullMove << std::endl;
            std::cout << "Castling: " << (int)currentState.castling << std::endl;
            std::cout << std::endl;
        }

        uint64_t get_occupancy(const State& state) const{
            uint64_t occ = 0ULL;
            for (const PairBitboard& pb : state.bitboards){
                occ |= pb.get<WHITE>() | pb.get<BLACK>();
            }
            return occ;
        }

        // TODO - simple check for some types of the moves.
        //      - For example pins -- can be detected with ray
        //      - King moves to enemy attacks -> illegal - DONE
        //      - King castling -> are squares attacked [?] - DONE
        template<Color us>
        bool is_illegal(const State& state, const Move& move){
            if (move.special_type() == Move::CASTLE){
                const uint64_t occ = get_occupancy(state);
                const bool kingSide = move.from() < move.to();

                if (kingSide && !state.is_castling_set<us, false>())
                    return false;

                if (!kingSide && !state.is_castling_set<us, true>())
                    return false;

                bool result = Movegen::is_square_attacked<us>(state, move.from(), occ);
                result |= Movegen::is_square_attacked<us>(state, kingSide ? move.from() + 1 : move.from() - 1, occ);
                result |= Movegen::is_square_attacked<us>(state, move.to(), occ);
                return result;
            }
            else if (state.pieceMap[move.from()] == KING)
                return Movegen::is_square_attacked<us>(state, move.to());

            // TODO pins.
            return false;
        }

        // TODO -- will be used in datagen -- oneday.
        std::string get_fen(){}
    };
}

#endif //SIGMOID_BOARD_HPP
