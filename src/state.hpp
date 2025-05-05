#ifndef SIGMOID_STATE_HPP
#define SIGMOID_STATE_HPP

#include <array>
#include <cstdint>

#include "piece.hpp"
#include "mailbox.hpp"

namespace Sigmoid{
    struct State{
        std::array<uint64_t, 12> bitboards;
        MailBox mailBox;

        uint8_t enPassantSquare = 0;
        uint64_t zobristKey = 0ULL;
        uint64_t halfMove = 0, fullMove = 0;
        uint8_t castling = 0;
    };
}

#endif //SIGMOID_STATE_HPP
