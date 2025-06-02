#ifndef SIGMOID_THREAD_HPP
#define SIGMOID_THREAD_HPP

#include "board.hpp"
#include "search.hpp"

namespace Sigmoid {

    struct Thread {
        const int id;
        Board board;
        TranspositionTable* tt;

        Thread(int id, Board board, TranspositionTable* tt) : id(id), board(board), tt(tt){}

        bool time_out() {
            // TODO just check time.
            return false;
        }

        SearchResult iterative_deepening(){
            // TODO
        }

        template<NodeType nodeType>
        int negamax(int alpha, int beta) {
            if (board.is_draw())
                return 0;
        }

        //template<NodeType nodeType>
        void q_search() {

        }

    };
}

#endif //SIGMOID_THREAD_HPP
