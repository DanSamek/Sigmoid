#ifndef SIGMOID_HELPER_HPP
#define SIGMOID_HELPER_HPP

#include <map>

#include "piece.hpp"

namespace Sigmoid {
    inline int get_square(int rank, int file){
        return rank * 8 + file;
    }

    static inline const std::map<char, Piece> LETTER_PIECE_MAP = {
            {'p', PAWN},
            {'n', KNIGHT},
            {'b', BISHOP},
            {'r', ROOK},
            {'q', QUEEN},
            {'k', KING}
    };


    [[nodiscard]] static inline std::string square_to_uci(uint8_t square){
        int file = square / 8;
        int rank = square % 8;

        std::ostringstream oss;
        oss << (char)(rank + 'a') << ((7 - file) + 1);
        std::string result = oss.str();
        return result;
    }
}

#endif //SIGMOID_HELPER_HPP
