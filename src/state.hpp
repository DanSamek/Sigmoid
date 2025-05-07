#ifndef SIGMOID_STATE_HPP
#define SIGMOID_STATE_HPP

#include <array>
#include <cstdint>
#include <map>

#include "piece.hpp"
#include "mailbox.hpp"
#include "constants.hpp"
#include "pairbitboard.hpp"

namespace Sigmoid{
    struct State{
        std::array<PairBitboard, 6> bitboards;
        MailBox mailBox;

        uint8_t enPassantSquare = 0;
        uint64_t zobristKey = 0ULL;
        uint64_t halfMove = 0, fullMove = 0;

        uint8_t castling = 0;

        void reset(){
            for(PairBitboard& bb : bitboards)
                bb.clear();

            zobristKey = castling = halfMove = fullMove = 0;
            enPassantSquare = NO_EN_PASSANT_SQUARE;
            mailBox.clear();
        }

        template<Color color>
        void set_bit(int square, Piece piece){
            bitboards[piece].set_bit<color>(square);
        }

        template<Color color>
        bool get_bit(int square, Piece piece){
            return bitboards[piece].get<color>(square);
        }

        // K [1 << 0], Q [1 << 1], k [1 << 2], q [1 << 3]
        enum Castling{
            K = 1,
            Q = 1 << 1,
            k = 1 << 2,
            q = 1 << 3
        };

        static inline const std::map<char, int> castlingIndexes = {{'K', 0}, {'Q', 1}, {'k', 2}, {'q', 3}};

        void set_casting_index(char c){
            set_nth_bit(castling, castlingIndexes.at(c));
        }

        template<Color us>
        uint8_t get_k_castling(){
            return us == Color::white() ? K : k;
        }

        template<Color us>
        uint8_t get_q_castling(){
            return us == Color::white() ? Q : q;
        }

        template<Color us, bool Q>
        bool is_castling_set(){
            if constexpr (Q){
                return (get_q_castling<us>() & castling) != 0;
            }
            else{
                return (get_k_castling<us>() & castling) != 0;
            }
        }

    };
}

#endif //SIGMOID_STATE_HPP
