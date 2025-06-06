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

        [[nodiscard]] bool is_time_out() const {
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
            constexpr bool pv_node = root_node || nodeType == PV;

            if (is_time_out())
                return MIN_VALUE;

            if (board.is_draw())
                return DRAW;

            if (stack->ply >= MAX_PLY)
                return board.eval();
            /*
            auto [entry, hit] = tt->probe(board.currentState.zobristKey);
            // TT - cutoffs.
            if (!pv_node && hit && entry.depth >= depth){
                int16_t eval;
                if (std::abs(entry.eval) >= CHECKMATE_BOUND)
                    eval = entry.eval - stack->ply * (entry.eval / std::abs(entry.eval));
                else
                    eval = entry.eval;

                if (entry.flag == EXACT)
                    return eval;
                // alpha stored -> alpha >= beta.
                else if (entry.flag == LOWER_BOUND && beta <= entry.eval)
                    return eval;
                // beta stored -> alpha >= beta.
                else if (entry.flag == UPPER_BOUND && alpha >= entry.eval)
                    return eval;
            }
            */
            if (depth == 0)
                return q_search(alpha, beta, stack);

            const bool in_check = board.in_check();

            MoveList<false> ml(&board);
            Move move;
            int move_count = 0;

            int16_t best_value = MIN_VALUE;
            //Flag tt_flag = UPPER_BOUND;
            //Move best_move;

            int16_t value;
            while ((move = ml.get()) != Move::none()){
                if (!board.make_move(move))
                    continue;

                //tt->prefetch(board.currentState.zobristKey);
                move_count++;
                result.nodesVisited++;

                // Late moves "reductions"
                if (pv_node && move_count == 1){
                    value = static_cast<int16_t>(-negamax<PV>(depth - 1, -beta, -alpha, stack + 1));
                }
                else{
                    int r = 0;
                    if (move_count > 3)
                        r += 128;

                    int reduced_depth = std::max(depth - 1 - r / 128, 0);
                    value = static_cast<int16_t>(-negamax<NONPV>(reduced_depth, -alpha - 1, -alpha, stack + 1));
                    // If move is looking promising, search in a full depth.
                    if (value > alpha && r)
                        value = static_cast<int16_t>(-negamax<NONPV>(depth - 1, -alpha - 1, -alpha, stack + 1));

                    bool full_search = value > alpha;
                    if (full_search && pv_node)
                        value = static_cast<int16_t>(-negamax<PV>(depth - 1, -beta, -alpha, stack + 1));
                    else if(full_search)
                        value = static_cast<int16_t>(-negamax<NONPV>(depth - 1, -beta, -alpha, stack + 1));
                }

                board.undo_move();

                if (is_time_out())
                    return MIN_VALUE;

                if (value > best_value) {
                    best_value = value;
                    //best_move = move;

                    if constexpr (root_node) {
                        result.bestMove = move;
                    }

                    if (value > alpha){
                        alpha = value;
                        //tt_flag = EXACT;
                    }

                    if (value >= beta){
                        //tt_flag = LOWER_BOUND;
                        break;
                    }
                }
            }

            if (move_count == 0 && in_check)
                return -CHECKMATE + stack->ply;
            else if (move_count == 0)
                return DRAW;

            //tt->store(board.currentState.zobristKey, best_move, tt_flag, depth, best_value);
            return best_value;
        }

        //template<NodeType nodeType>
        int16_t q_search(int16_t alpha, int16_t beta, StackItem* stack) {
            int16_t best_value = board.eval();
            if (stack->ply >= MAX_PLY)
                return best_value;

            if (best_value >= beta)
                return best_value;
            if (best_value > alpha)
                alpha = best_value;

            MoveList<true> ml(&board);
            Move move;
            while ((move = ml.get()) != Move::none()){
                if (!board.make_move(move))
                    continue;

                result.nodesVisited++;
                int16_t value = static_cast<int16_t>(-q_search(-beta, -alpha, stack + 1));

                board.undo_move();

                if (is_time_out())
                    return MIN_VALUE;

                if (value > best_value) {
                    best_value = value;

                    if (value > alpha)
                        alpha = value;

                    if (value >= beta)
                        break;
                }
            }

            return best_value;
        }
    };
}

#endif //SIGMOID_WORKER_HPP
