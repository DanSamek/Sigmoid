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

        // Called before every search.
        void load_state(Board b, TranspositionTable* t, WorkerHelper* wh, Timer* tm, int sd){
            board = std::move(b);
            tt = t;
            workerHelper = wh;
            timer = tm;
            searchDepth = sd;
            result = SearchResult();
        }

        void new_game(){
            prepare_for_search();
        }

        bool is_time_out() {
            if (searchDepth != MAX_PLY - 1)
                return false;

            return timer->is_time_out();
        }

        void iterative_deepening() {

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

            int16_t eval;
            for (int depth = 1; depth <= searchDepth; depth++){
                if (depth <= 5){
                    eval = negamax<ROOT>(depth, MIN_VALUE, MAX_VALUE, root);

                    if (is_time_out())
                        break;

                    result.score = eval;
                    workerHelper->enter_search_result(depth, result);
                    continue;
                }

                int16_t delta = 20;
                int16_t alpha = std::max(MIN_VALUE, (int16_t)(eval - delta));
                int16_t beta = std::min(MAX_VALUE, (int16_t)(eval + delta));

                while (true){
                    eval = negamax<ROOT>(depth, alpha, beta, root);

                    if (eval <= alpha && eval > -CHECKMATE_BOUND){
                        // beta = (eval + beta) / 2; todo try, if pass.
                        alpha -= delta;
                    }
                    else if (eval >= beta && eval < CHECKMATE_BOUND){
                        // alpha = (eval + alpha) / 2; todo try, if pass.
                        beta += delta;
                    }
                    else{
                        if (!is_time_out()){
                            result.score = eval;
                            workerHelper->enter_search_result(depth, result);
                        }
                        break;
                    }

                    delta *= 2;
                    if (delta >= 1000){
                        alpha = MIN_VALUE;
                        beta = MAX_VALUE;
                    }
                }

                if (is_time_out())
                    break;
            }
        }

        template<NodeType nodeType>
        int16_t negamax(int depth, int16_t alpha, int16_t beta, StackItem* stack) {
            constexpr bool root_node = nodeType == ROOT;
            constexpr bool pv_node = nodeType != NONPV;

            if (is_time_out())
                return MIN_VALUE;

            if (board.is_draw())
                return DRAW;

            if (stack->ply >= MAX_PLY)
                return board.eval();

            auto [entry, tt_hit] = tt->probe(board.key());
            if (!pv_node && tt_hit && entry.depth >= depth){

                auto correct_tt_eval = [&stack](int16_t eval)-> int16_t {
                    auto abs_eval = std::abs(eval);
                    if(abs_eval >= CHECKMATE_BOUND)
                        eval -= stack->ply * (abs_eval/eval);
                    return eval;
                };

                int16_t corrected_eval = correct_tt_eval(entry.eval);
                if (entry.flag == EXACT)
                    return corrected_eval;
                if (entry.flag == UPPER_BOUND && entry.eval <= alpha)
                    return corrected_eval;
                if (entry.flag == LOWER_BOUND && entry.eval >= beta)
                    return corrected_eval;
            }

            if (depth <= 0)
                return q_search(alpha, beta, stack);

            const int16_t static_eval = board.eval();
            const bool in_check = board.in_check();

            if (!in_check) {
                // Reverse futility pruning.
                // If eval is really good, that even with big margin beats beta, return static eval.
                if (!pv_node && depth <= 8 && static_eval >= beta + 100 * depth)
                    return static_eval;
            }


            MoveList<false> ml(&board, &mainHistory, &entry.move, &continuationHistory, stack);
            Move move;
            int move_count = 0;
            Move best_move = NO_MOVE;
            int16_t best_value = MIN_VALUE;
            std::vector<Move> quiet_moves;

            TTFlag flag = UPPER_BOUND;
            while ((move = ml.get()) != NO_MOVE){

                const bool is_capture = board.is_capture(move);
                if (!board.make_move(move))
                    continue;

                move_count++;
                result.nodesVisited++;
                tt->prefetch(board.key());

                stack->currentMove = move;
                int16_t value;
                if (move_count == 1){
                    value = -negamax<nodeType>(depth - 1, -beta, -alpha, stack + 1);
                }
                else{
                    value = -negamax<NONPV>(depth - 1, -alpha - 1, -alpha, stack + 1);

                    if (value > alpha && pv_node)
                        value = -negamax<PV>(depth - 1, -beta, -alpha, stack + 1);
                }
                stack->currentMove = NO_MOVE;
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

            if (best_move != NO_MOVE && !board.is_capture(best_move)) {
                update_quiet_histories(best_move, quiet_moves);
                update_continuation_histories(stack, best_move, quiet_moves);
            }

            if (move_count == 0 && in_check)
                return -CHECKMATE + stack->ply;
            else if (move_count == 0)
                return DRAW;

            tt->store(board.key(), best_move, flag, depth, best_value, stack->ply);
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
            while ((move = ml.get()) != NO_MOVE){
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
            apply_gravity<int16_t>(mainHistory[board.whoPlay][bestMove.from()][bestMove.to()], 700, MainHistory::maxValue);

            for (const Move& move: quietMoves)
                apply_gravity<int16_t>(mainHistory[board.whoPlay][move.from()][move.to()], -250, MainHistory::maxValue);
        }

        void update_continuation_histories(const StackItem* stack, const Move& bestMove, const std::vector<Move>& quietMoves){
            update_continuation_histories_move(stack, bestMove, 500);
            for (const Move& move : quietMoves)
                update_continuation_histories_move(stack, move, -300);
        }

        void update_continuation_histories_move(const StackItem* stack, const Move& move, int bonus){
            for (int n_ply = 1; n_ply <= CONT_HIST_MAX_PLY; n_ply++){
                const Move& previous_move = (stack - n_ply)->currentMove;
                if (previous_move == NO_MOVE)
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
