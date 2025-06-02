#ifndef SIGMOID_SEARCH_HPP
#define SIGMOID_SEARCH_HPP

#include "move.hpp"

namespace Sigmoid {
    enum NodeType{
        Root,
        PV,
        NonPv
    };

    struct SearchResult {
        Move bestMove;
        uint16_t score;
        // PV TODO.
    };

    struct StackItem{
        Move currentMove;
        int8_t ply;
    };
}

#endif //SIGMOID_SEARCH_HPP
