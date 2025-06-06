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

        MainHistory mainHistory;

        bool readyTables = false;
        std::array<std::array<int, MAX_POSSIBLE_MOVES>, MAX_PLY> lmrTable;

        Worker(Board board, TranspositionTable* tt, WorkerHelper* wh, Timer* timer ,int searchDepth) :
            board(std::move(board)), tt(tt), workerHelper(wh), timer(timer), searchDepth(searchDepth) {}

        bool is_time_out() {
            if (searchDepth != MAX_PLY - 1)
                return false;

            return timer->is_time_out();
        }

        void iterative_deepening() {
            prepare_for_search();

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

            if (depth == 0)
                return q_search(alpha, beta, stack);

            if (stack->ply >= MAX_PLY)
                return board.eval();

            const bool in_check = board.in_check();

            MoveList<false> ml(&board, &mainHistory);
            Move move;
            int move_count = 0;
            Move best_move = Move::none();
            int16_t best_value = MIN_VALUE;
            int16_t value;
            std::vector<Move> quiet_moves;

            while ((move = ml.get()) != Move::none()) {
                const bool is_capture = board.is_capture(move);
                if (!board.make_move(move))
                    continue;

                move_count++;
                result.nodesVisited++;
                if (pv_node && move_count == 1){
                    value = static_cast<int16_t>(-negamax<PV>(depth - 1, -beta, -alpha, stack + 1));
                }
                else{
                    int r = lmrTable[depth - 1][move_count - 1];
                    int reduced_depth = std::max(depth - 1 - r / 128, 0);

                    // Try search with a reduced depth.
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
                    best_move = move;

                    if constexpr (root_node) {
                        result.bestMove = move;
                    }

                    if (value > alpha)
                        alpha = value;

                    if (value >= beta)
                        break;
                }

                if (!is_capture && move != best_move)
                    quiet_moves.emplace_back(move);
            }

            if (best_move != Move::none())
                update_quiet_histories(best_move, quiet_moves);

            if (move_count == 0 && in_check)
                return -CHECKMATE + stack->ply;
            else if (move_count == 0)
                return DRAW;

            // TODO TT store.
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

        void update_quiet_histories(Move bestMove, const std::vector<Move>& quietMoves){
            apply_gravity<int16_t>(mainHistory[board.whoPlay][bestMove.from()][bestMove.to()], 700);

            for (const Move& move: quietMoves)
                apply_gravity<int16_t>(mainHistory[board.whoPlay][move.from()][move.to()], -250);
        }

        void prepare_for_search(){
            for (auto& color : mainHistory)
                for (auto& from : color)
                    for (auto& to: from)
                        to = 0;

            if (readyTables)
                return;

            for(int depth = 1; depth <= MAX_PLY; depth++)
                for(int moveCnt = 1; moveCnt <= MAX_POSSIBLE_MOVES; moveCnt++)
                    lmrTable[depth - 1][moveCnt - 1] = int(round(0.5 + (log(depth) * log(moveCnt) / 3)) * 128);

            readyTables = true;
        }
    };
}

#endif //SIGMOID_WORKER_HPP
