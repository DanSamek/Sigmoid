#ifndef SIGMOID_PIECE_HPP
#define SIGMOID_PIECE_HPP

#include <map>
#include <cctype>

#include "color.hpp"


namespace Sigmoid{
    enum Piece {
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING,
        NONE = 0xF
    };

    static inline const std::map<Piece, char> PIECE_MAP_CHAR = {{NONE, ' '}, {PAWN, 'p'}, {KNIGHT, 'n'}, {BISHOP, 'b'}, {ROOK, 'r'}, {QUEEN, 'q'}, {KING, 'k'}};

    template<Color color>
    inline char piece_char(Piece p){
        char c = PIECE_MAP_CHAR.at(p);
        if constexpr (color == WHITE) c = toupper(c);
        return c;
    }
}

#endif //SIGMOID_PIECE_HPP
