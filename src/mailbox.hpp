#ifndef SIGMOID_MAILBOX_HPP
#define SIGMOID_MAILBOX_HPP

#include <array>
#include <cstdint>
#include <cassert>

#include "piece.hpp"

namespace Sigmoid {
    struct MailBox {
        std::array<uint32_t, 8> data;

        MailBox(){
            clear();
        }

        Piece at(int square){
            int row = square / 8;
            int col = square % 8;

            Piece result = (Piece)((data[row] >> correct_shift(col)) & 0xF);
            return result;
        }

        void set_piece(int square, Piece piece){
            int row = square / 8;
            int col = square % 8;

            int shifted_col = correct_shift(col);
            assert(((data[row] >> shifted_col) & Piece::NONE) == Piece::NONE);

            data[row] ^= (~(piece) & 0xF) << shifted_col;
        }

        void clear(){
            for (uint32_t& item : data){
                item = 0xFFFFFFFF;
            }
        }

    private:
        int correct_shift(int col){
            return col * 4;
        }
    };

}


#endif //SIGMOID_MAILBOX_HPP
