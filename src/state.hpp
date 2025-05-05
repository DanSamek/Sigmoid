#ifndef SIGMOID_STATE_HPP
#define SIGMOID_STATE_HPP

#include <array>
#include <cstdint>

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

        /*
         1 =     r  k [black]
         1 << 2  r  k [black]
         1 << 3  R  K [white]
         1 << 4  K  R [white]
         */
        uint8_t castling = 0;

        void reset(){
            for(PairBitboard& bb : bitboards)
                bb.clear();

            zobristKey = castling = halfMove =fullMove = 0;
            enPassantSquare = EN_PASSANT_SQUARE;
            mailBox.clear();
        }
    };
}

#endif //SIGMOID_STATE_HPP
