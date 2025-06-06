#ifndef SIGMOID_WORKER_HPP
#define SIGMOID_WORKER_HPP

#include <utility>

#include "board.hpp"
#include "search.hpp"
#include "worker_helper.hpp"
#include "tt.hpp"
#include "movelist.hpp"
#include "timer.hpp"

namespace Sigmoid {

    struct Worker {
        Board board;
        TranspositionTable* tt;
        WorkerHelper* workerHelper;
        SearchResult result;
        Timer* timer;
        int searchDepth;

        Worker(Board board, TranspositionTable* tt, WorkerHelper* wh, Timer* timer ,int searchDepth) :
            board(std::move(board)), tt(tt), workerHelper(wh), timer(timer), searchDepth(searchDepth) {}

        bool is_time_out() {
            if (searchDepth != MAX_PLY - 1)
                return false;

            return timer->is_time_out();
        }

        void iterative_deepening() {
            StackItem stack[MAX_PLY + 1];
            StackItem* root = stack + 1;

            for (int i = 0; i < MAX_PLY - 1; i++){
                (root + i)->ply = i;
                (root + i)->currentMove = Move::none();
                (root + i)->excludedMove = Move::none();
            }

            for (int depth = 1; depth <= searchDepth; depth++){
                int16_t eval = negamax<ROOT>(depth, MIN_VALUE, MAX_VALUE, root);

                if (is_time_out())
                    break;

                result.score = eval;
                workerHelper->enter_search_result(depth, result);
            }
        }

        template<NodeType nodeType>
        int16_t negamax(int depth, int16_t alpha, int16_t beta, StackItem* stack) {
            constexpr bool root_node = nodeType == ROOT;

            if (is_time_out())
                return MIN_VALUE;

            if (board.is_draw())
                return DRAW;

            if (depth == 0)
                return board.eval();

            if (stack->ply >= MAX_PLY)
                return board.eval();

            const bool in_check = board.in_check();

            MoveList<false> mp(&board);
            Move move;
            int move_count = 0;
            int16_t best_value = MIN_VALUE;
            while ((move = mp.get()) != Move::none()){
                if (!board.make_move(move))
                    continue;

                move_count++;
                result.nodesVisited++;
                int16_t value = static_cast<int16_t>(-negamax<PV>(depth - 1, -beta, -alpha, stack + 1));

                board.undo_move();

                if (is_time_out())
                    return MIN_VALUE;

                if (value > best_value) {
                    best_value = value;
                    if constexpr (root_node) {
                        result.bestMove = move;
                    }

                    if (value > alpha)
                        alpha = value;

                    if (value >= beta)
                        break;
                }
            }

            if (move_count == 0 && in_check)
                return -CHECKMATE + stack->ply;
            else if (move_count == 0)
                return DRAW;

            // TODO TT store.
            return best_value;
        }

        //template<NodeType nodeType>
        /*
         * TODO
        int16_t q_search(int16_t alpha, int16_t beta) {

        }*/
    };
}

#endif //SIGMOID_WORKER_HPP
