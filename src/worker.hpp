#ifndef SIGMOID_WORKER_HPP
#define SIGMOID_WORKER_HPP

#include <utility>

#include "board.hpp"
#include "search.hpp"
#include "worker_helper.hpp"
#include "tt.hpp"
#include "movelist.hpp"
#include "timer.hpp"
#include "history.hpp"

namespace Sigmoid {

    struct Worker {
        Board board;
        TranspositionTable* tt;
        WorkerHelper* workerHelper;
        SearchResult result;
        Timer* timer;
        int searchDepth;

        MainHistory::type mainHistory;
        ContinuationHistory continuationHistory;

        Worker(Board board, TranspositionTable* tt, WorkerHelper* wh, Timer* timer ,int searchDepth) :
            board(std::move(board)), tt(tt), workerHelper(wh), timer(timer), searchDepth(searchDepth) {}

        bool is_time_out() {
            if (searchDepth != MAX_PLY - 1)
                return false;

            return timer->is_time_out();
        }

        void iterative_deepening() {
            prepare_for_search();

            StackItem stack[MAX_PLY + 2];
            StackItem* root = stack + 2;

            for (int i = 0; i < MAX_PLY - 1; i++){
                (root + i)->ply = i;
                (root + i)->currentMove = Move::none();
                (root + i)->excludedMove = Move::none();
            }

            for (int i = 0; i < 2; i++){
                (root - i)->currentMove = Move::none();
                (root - i)->excludedMove = Move::none();
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

            auto [entry, hit] = tt->probe(board.key());
            if (!pv_node && hit && entry.depth >= depth){
                // TODO TT cutoffs.
            }

            if (depth == 0)
                return q_search(alpha, beta, stack);

            const bool in_check = board.in_check();

            MoveList<false> ml(&board, &mainHistory, &entry.move, &continuationHistory, stack);
            Move move;
            int move_count = 0;
            Move best_move = Move::none();
            int16_t best_value = MIN_VALUE;
            std::vector<Move> quiet_moves;

            TTFlag flag = UPPER_BOUND;
            while ((move = ml.get()) != Move::none()){

                const bool is_capture = board.is_capture(move);
                if (!board.make_move(move))
                    continue;

                move_count++;
                result.nodesVisited++;
                tt->prefetch(board.key());

                stack->currentMove = move;
                int16_t value = static_cast<int16_t>(-negamax<PV>(depth - 1, -beta, -alpha, stack + 1));
                stack->currentMove = Move::none();

                board.undo_move();

                if (is_time_out())
                    return MIN_VALUE;

                if (value > best_value) {
                    best_value = value;
                    best_move = move;

                    if constexpr (root_node) {
                        result.bestMove = move;
                    }

                    if (value > alpha){
                        alpha = value;
                        flag = EXACT;
                    }

                    if (value >= beta){
                        flag = LOWER_BOUND;
                        break;
                    }
                }

                if (!is_capture && move != best_move)
                    quiet_moves.emplace_back(move);
            }

            if (best_move != Move::none() && !board.is_capture(best_move)){
                update_quiet_histories(best_move, quiet_moves);
                if (best_value >= beta)
                    update_continuation_histories(stack, best_move, quiet_moves);
            }

            if (move_count == 0 && in_check)
                return -CHECKMATE + stack->ply;
            else if (move_count == 0)
                return DRAW;

            tt->store(board.key(), best_move, flag, depth, best_value);
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

        void update_quiet_histories(const Move& bestMove, const std::vector<Move>& quietMoves){
            apply_gravity<int16_t>
                    (mainHistory[board.whoPlay][bestMove.from()][bestMove.to()], 700, MainHistory::maxValue);

            for (const Move& move: quietMoves)
                apply_gravity<int16_t>
                        (mainHistory[board.whoPlay][move.from()][move.to()], -250, MainHistory::maxValue);
        }

        void update_continuation_histories(const StackItem* stack, const Move& bestMove, const std::vector<Move>& quietMoves){
            update_continuation_histories_move(stack, bestMove, 350);

            for (const Move& move : quietMoves)
                update_continuation_histories_move(stack, move, -150);
        }

        void update_continuation_histories_move(const StackItem* stack, const Move& move, int bonus){
            for (int n_ply = 1; n_ply <= CONT_HIST_MAX_PLY; n_ply++){
                const Move& previous_move = (stack - n_ply)->currentMove;
                if (previous_move == Move::none())
                    break;

                int16_t& entry = continuationHistory[n_ply - 1][previous_move.from()][previous_move.to()][move.from()][move.to()];
                apply_gravity<int16_t>(entry, bonus, ContinuationHistoryEntry::maxValue);
            }
        }

        void prepare_for_search(){
            for (auto& color : mainHistory)
                for (auto& from : color)
                    for (auto& to: from)
                        to = 0;

            for (auto& n_ply : continuationHistory)
                for (auto& prev_from: n_ply)
                    for (auto& prev_to: prev_from)
                        for (auto& curr_from: prev_to)
                            for (auto& curr_to: curr_from)
                                curr_to = 0;
        }
    };
}

#endif //SIGMOID_WORKER_HPP
