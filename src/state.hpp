#ifndef SIGMOID_STATE_HPP
#define SIGMOID_STATE_HPP

#include <array>
#include <cstdint>
#include <map>

#include "piece.hpp"
#include "constants.hpp"
#include "pairbitboard.hpp"

namespace Sigmoid{
    struct State{
        std::array<PairBitboard, 6> bitboards;
        std::array<Piece, 64> pieceMap;
        uint8_t enPassantSquare = 0;
        uint64_t zobristKey = 0ULL;
        uint16_t halfMove = 0, fullMove = 1;

        uint8_t castling = 0;

        void reset(){
            for(PairBitboard& bb : bitboards)
                bb.clear();

            for (Piece& p : pieceMap)
                p = NONE;

            zobristKey = castling = halfMove = fullMove = 0;
            enPassantSquare = NO_EN_PASSANT_SQUARE;
        }

        template<Color color>
        void set_bit(int square, Piece piece){
            bitboards[piece].set_bit<color>(square);
        }

        template<Color color>
        bool get_bit(int square, Piece piece) const{
            return bitboards[piece].get<color>(square);
        }

        // K [1 << 0], Q [1 << 1], k [1 << 2], q [1 << 3]
        enum Castling{
            K = 1,
            Q = 1 << 1,
            k = 1 << 2,
            q = 1 << 3
        };

        static inline constexpr int K_CASTLING_BIT = 0;
        static inline constexpr int Q_CASTLING_BIT = 1;

        static inline constexpr int k_CASTLING_BIT = 2;
        static inline constexpr int q_CASTLING_BIT = 3;

        static inline const std::map<char, int> castlingIndexes = {{'K', 0}, {'Q', 1}, {'k', 2}, {'q', 3}};

        void set_casting_index(char c){
            set_nth_bit(castling, castlingIndexes.at(c));
        }

        void disable_castling_index(int bit){
            pop_nth_bit(castling, bit);
        }

        template<Color us>
        uint8_t get_k_castling() const{
            constexpr uint8_t result = us == WHITE ? K : k;
            return result;
        }

        template<Color us>
        uint8_t get_q_castling() const{
            constexpr uint8_t result = us == WHITE ? Q : q;
            return result;
        }

        template<Color us>
        void disable_castling(){
            uint8_t disable = get_k_castling<us>() | get_q_castling<us>();
            castling &= ~disable;
        }

        template<Color us>
        bool is_some_castling_set() const{
            return is_castling_set<us, true>() || is_castling_set<us, false>();
        }

        template<Color us, bool Q>
        bool is_castling_set() const{
            if constexpr (Q){
                return (get_q_castling<us>() & castling) != 0;
            }
            else{
                return (get_k_castling<us>() & castling) != 0;
            }
        }

        template<Color us>
        void disable_castling_rook_move(int fromSq){
            int index;
            if constexpr (us == WHITE)
                index = fromSq == 63 ? K_CASTLING_BIT : fromSq == 56 ? Q_CASTLING_BIT : -1;
            else
                index = fromSq == 7 ? k_CASTLING_BIT : fromSq == 0 ? q_CASTLING_BIT : -1;

            if (index != -1)
                disable_castling_index(index);
        }
    };
}

#endif //SIGMOID_STATE_HPP
