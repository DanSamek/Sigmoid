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
#include "zobrist.hpp"
#include "./nnue/nnue.hpp"

namespace Sigmoid {
    struct Board{
        std::array<State, STACK_SIZE_P1> stateStack;
        int ply = 0;
        Color whoPlay;
        State currentState;
        NNUE nnue;

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

        template<Color us>
        bool make_move(const Move& move) {
            if (is_illegal<us>(currentState, move))
                return false;

            constexpr Color op = ~us;
            State new_state = currentState;
            uint64_t prev_castling = new_state.castling;

            if (currentState.enPassantSquare != NO_SQUARE)
                new_state.zobristKey ^= Zobrist::epSquares[currentState.enPassantSquare];

            new_state.zobristKey ^= Zobrist::sideToMove;
            new_state.enPassantSquare = NO_SQUARE;

            new_state.fullMove += us == BLACK;

            const bool is_cap = is_capture(move);
            const Piece piece = at(move.from());
            const int from = move.from();
            const int to = move.to();
            const Piece promo_piece = move.promo_piece();
            const Move::SpecialType special_type = move.special_type();

            Piece captured = NONE;
            Piece to_piece = piece;

            if (is_cap)
            {
                captured = new_state.pieceMap[to];
                assert(captured != NONE);
                disable_cap_castling<us>(new_state, move);
                new_state.bitboards[captured].pop_bit<op>(to);

                new_state.zobristKey ^= Zobrist::pieceKeys[~us][captured][to];
                new_state.halfMove = 0;
            }
            else{
                new_state.halfMove = piece == PAWN ? 0 : new_state.halfMove + 1;
                switch (special_type) {
                    case Move::EN_PASSANT:
                        handle_ep<us>(new_state, to);
                        break;
                    case Move::CASTLE:
                        handle_castling<us>(new_state, from, to);
                        break;
                    case Move::NONE:
                        if (piece == Piece::PAWN && abs(from - to) > 8) {
                            new_state.enPassantSquare = us == WHITE ? from - 8 : from + 8;
                            new_state.zobristKey ^= Zobrist::epSquares[new_state.enPassantSquare];
                        }
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

            new_state.zobristKey ^= Zobrist::pieceKeys[us][piece][from];
            new_state.zobristKey ^= Zobrist::pieceKeys[us][promo_piece == NONE ? piece : promo_piece][to];

            disable_castling<us>(new_state, piece, move);

            if (prev_castling != new_state.castling) {
                new_state.zobristKey ^= Zobrist::castlingKeys[prev_castling];
                new_state.zobristKey ^= Zobrist::castlingKeys[new_state.castling];
            }

            uint64_t tmp = new_state.bitboards[KING].get<us>();
            int new_king_pos = bit_scan_forward_pop_lsb(tmp);
            if (Movegen::is_square_attacked<us>(new_state, new_king_pos))
                return false;


            nnue.push();

            if (is_cap)
            {
                assert(captured != NONE);
                nnue.sub(~us, captured, to);
            }
            else{
                switch (special_type) {
                    case Move::EN_PASSANT:
                        handle_ep_nnue<us>(to);
                        break;
                    case Move::CASTLE:
                        handle_castling_nnue<us>(from, to);
                        break;
                    default:
                        break;
                }
            }
            if (promo_piece != NONE){
                nnue.sub(us, piece, from);
                nnue.add(us, to_piece, to);
            }
            else{
                nnue.move_piece(us, piece, from, to);
            }

            whoPlay = ~whoPlay;
            stateStack[ply] = currentState;
            currentState = new_state;

            ply++;
            return true;
        }

        void undo_move(){
            assert(ply >= 1);
            ply--;
            currentState = stateStack[ply];
            whoPlay = ~whoPlay;

            nnue.pop();
        }

        bool in_check() {
            uint64_t king_bb;
            king_bb = whoPlay == BLACK ?
                    currentState.bitboards[KING].get<BLACK>() : currentState.bitboards[KING].get<WHITE>();

            int king_square = bit_scan_forward_pop_lsb(king_bb);
            return whoPlay == BLACK ?
                Movegen::is_square_attacked<BLACK>(currentState, king_square)
                        : Movegen::is_square_attacked<WHITE>(currentState, king_square);
        }

        int16_t eval() {
            return whoPlay == WHITE ? nnue.eval<WHITE>() : nnue.eval<BLACK>();
        }

        void load_from_fen(std::string fen){
            nnue.reset();
            currentState.reset();
            ply = 0;

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

                nnue.add(isupper(c) ? WHITE : BLACK, piece, square);
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

            currentState.zobristKey = Zobrist::get_key(currentState, whoPlay);
        }

        // Only for debug.
        void print_state() const {
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


        // rnbqkbnr/pppppppp/8/8/52P3/8/PPPP49PPP/RNBQKBNR b KQkq e3 0 1
        [[nodiscard]] std::string get_fen(){
            int no_piece_cnt = 0;
            std::ostringstream oss;
            for (int square = 0; square < 64; square++){
                Piece pc = at(square);
                if (pc == NONE){
                    no_piece_cnt++;
                }
                else{
                    if (no_piece_cnt) {
                        oss << char('0' + no_piece_cnt);
                        no_piece_cnt = 0;
                    }
                    uint64_t wbb = currentState.bitboards[pc].get<WHITE>();
                    if (get_nth_bit(wbb, square))
                        oss << piece_char<WHITE>(pc);
                    else
                        oss << piece_char<BLACK>(pc);
                }

                if ((square + 1) % 8 == 0){
                    if (no_piece_cnt)
                        oss << (char)('0' + no_piece_cnt);
                    no_piece_cnt = 0;

                    if (square != 63)
                        oss << "/";
                }
            }

            oss << " ";
            oss << (whoPlay == WHITE ? 'w' : 'b') << " ";

            oss << currentState.castling_str() << " ";

            if (currentState.enPassantSquare == NO_SQUARE)
                oss << "- ";
            else
                oss << square_to_uci(currentState.enPassantSquare) << " ";

            oss << currentState.halfMove << " " << currentState.fullMove;

            std::string result = oss.str();
            return result;
        }

        [[nodiscard]] bool is_draw() const{
            if (currentState.halfMove >= 100)
                return true;

            if (is_insufficient_material<WHITE>() && is_insufficient_material<BLACK>())
                return true;

            int hit_count = 0;
            for (int i = 0; i < ply; i++){
                if (stateStack[i].zobristKey == currentState.zobristKey)
                    hit_count++;
                if (hit_count == 2)
                    return true;
            }
            return false;
        }

    private:

        template<Color us>
        [[nodiscard]] bool is_insufficient_material() const{
            if (currentState.bitboards[QUEEN].get<us>()
                || currentState.bitboards[ROOK].get<us>()
                || currentState.bitboards[PAWN].get<us>())
                return false;

            uint64_t bb = currentState.bitboards[BISHOP].get<us>();
            const int bishop_cnt = count_bits(bb);
            if (bishop_cnt >= 2) return false;

            bb = currentState.bitboards[KNIGHT].get<us>();
            const int knight_cnt = count_bits(bb);
            if (knight_cnt >= 2) return false;

            return !(knight_cnt == 1 && bishop_cnt == 1);
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

            state.zobristKey ^= Zobrist::pieceKeys[us][piece][from];
            state.zobristKey ^= Zobrist::pieceKeys[us][piece][to];
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

            state.zobristKey ^= Zobrist::pieceKeys[~us][PAWN][enemy_pawn_square];
        }

        inline static uint64_t get_occupancy(const State& state) {
            uint64_t occ = 0ULL;
            for (const PairBitboard& pb : state.bitboards){
                occ |= pb.get<WHITE>() | pb.get<BLACK>();
            }
            return occ;
        }

        template<Color us>
        bool is_illegal(const State& state, const Move& move){
            if (move.special_type() == Move::CASTLE){
                const uint64_t occ = get_occupancy(state);
                const bool kingSide = move.from() < move.to();

                bool result = Movegen::is_square_attacked<us>(state, move.from(), occ);
                result |= Movegen::is_square_attacked<us>(state, kingSide ? move.from() + 1 : move.from() - 1, occ);
                result |= Movegen::is_square_attacked<us>(state, move.to(), occ);
                return result;
            }
            else if (state.pieceMap[move.from()] == KING)
                return Movegen::is_square_attacked<us>(state, move.to());

            return false;
        }


        template<Color us>
        void move_piece_nnue(int from, int to, Piece piece){
            nnue.move_piece(us, piece, from, to);
        }

        template<Color us>
        void handle_castling_nnue(int from, int to){
            const bool king_side = from < to;
            if (king_side)
                move_piece_nnue<us>(to + 1, to - 1, ROOK);
            else
                move_piece_nnue<us>(to - 2, to + 1, ROOK);
        }

        template<Color us>
        void handle_ep_nnue(int to){
            const int enemy_pawn_square = us == WHITE ? to + 8 : to - 8;
            nnue.sub(~us, PAWN, enemy_pawn_square);
        }
    };
}

#endif //SIGMOID_BOARD_HPP
