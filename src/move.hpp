
#ifndef SIGMOID_MOVE_HPP
#define SIGMOID_MOVE_HPP

#include <stdint.h>
#include <cassert>

namespace Sigmoid{
    // [en-passant|castle|promo[KNIGHT, BISHOP, ROOK, QUEEN], NONE]
    // none = 0;
    // en-passant = 1;
    // castle = 2
    // promo = <3, 6>
    // [8 - 3b][to - 6b][from - 6b]
    struct Move{
        enum SpecialType{
            NONE = 0,
            EN_PASSANT = 1,
            CASTLE = 2,
            PROMO_KNIGHT = 3,
            PROMO_BISHOP = 4,
            PROMO_ROOK = 5,
            PROMO_QUEEN = 6
        };

        uint16_t data = 0;

        uint8_t from(){
            uint8_t result = data & SQ_MASK;
            return result;
        }

        uint8_t to(){
            uint8_t result = (data >> 6) & SQ_MASK;
            return result;
        }

        SpecialType special_type(){
            SpecialType result = (SpecialType)(data >> 12);
            return result;
        }

        void set_data(int from, int to, SpecialType type = NONE){
            data = 0;
            data |= from;
            data |= to << 6;
            data |= type << 12;
        }

        static Move none(){
            return Move();
        }

    private:
        static inline constexpr int SQ_MASK = 0x3F;
    };
}

#endif //SIGMOID_MOVE_HPP
