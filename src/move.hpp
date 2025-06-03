
#ifndef SIGMOID_MOVE_HPP
#define SIGMOID_MOVE_HPP

#include <stdint.h>
#include <cassert>
#include <sstream>

#include "piece.hpp"

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

        auto operator<=>(const Move&) const = default;

        Move() {}

        Move(int from, int to, SpecialType type = NONE){
            set_data(from, to, type);
        }

        bool operator==(const Move& move) const = default;

        uint8_t from() const{
            uint8_t result = data & SQ_MASK;
            return result;
        }

        uint8_t to() const{
            uint8_t result = (data >> 6) & SQ_MASK;
            return result;
        }

        SpecialType special_type() const{
            SpecialType result = (SpecialType)(data >> 12);
            return result;
        }

        Piece promo_piece() const{
            SpecialType tmp =  special_type();
            return tmp < 3 ? Piece::NONE : Piece(tmp - 2);
        }

        void set_data(int from, int to, SpecialType type = NONE){
            data = 0;
            data |= from;
            data |= to << 6;
            data |= type << 12;
        }

        inline static Move none(){
            return Move(0,0);
        }

        std::string to_uci(){
            std::ostringstream oss;
            auto sq_to_uci = [](uint8_t square)->std::string{
                int file = square / 8;
                int rank = square % 8;

                std::ostringstream oss;
                oss << (char)(rank + 'a') << ((7 - file) + 1);
                std::string result = oss.str();
                return result;
            };

            oss << sq_to_uci(from());
            oss << sq_to_uci(to());
            if (promo_piece() != Piece::NONE)
                oss << piece_char<BLACK>(promo_piece());

            std::string result = oss.str();
            return result;
        }

    private:
        static inline constexpr int SQ_MASK = 0x3F;
    };
}

#endif //SIGMOID_MOVE_HPP
