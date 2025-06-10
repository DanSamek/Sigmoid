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

        std::array<std::array<Move, MAX_PLY>, MAX_PLY> pvTable;
        std::array<uint8_t, MAX_PLY> pvLength;

        uint64_t nodesVisited = 0ULL;
    };

    struct StackItem {
        Move currentMove = Move::none();
        Move excludedMove = Move::none();
        int8_t ply = 0;
        bool can_null = false;
    };
}

#endif //SIGMOID_SEARCH_HPP
