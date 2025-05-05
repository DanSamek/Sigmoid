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
}

#endif //SIGMOID_HELPER_HPP
