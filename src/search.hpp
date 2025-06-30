#ifndef SIGMOID_SEARCH_HPP
#define SIGMOID_SEARCH_HPP

#include "move.hpp"

namespace Sigmoid {
    enum NodeType{
        ROOT,
        PV,
        NONPV
    };

    struct SearchResult {
        Move bestMove = Move::none();
        int16_t score = MIN_VALUE;

        std::array<std::array<Move, MAX_PLY + 1>, MAX_PLY + 1> pvTable;
        std::array<uint8_t, MAX_PLY + 1> pvLength;

        uint64_t nodesVisited = 0ULL;

        SearchResult(){
            for (uint8_t& length : pvLength)
                length = 0;
        }
    };

    struct StackItem {
        Move currentMove = Move::none();
        Move excludedMove = Move::none();
        int8_t ply = 0;
        bool can_null = true;
        Piece movedPiece = NONE;
        int16_t eval = MAX_VALUE;
        int cutoffCount = 0;
    };
}

#endif //SIGMOID_SEARCH_HPP
