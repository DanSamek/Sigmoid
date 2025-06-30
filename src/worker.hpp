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
        CaptureHistory::type captureHistory;
        KillerMoves killerMoves;

        static inline std::array<std::array<int16_t, MAX_POSSIBLE_MOVES>, MAX_PLY> lmrTable;
        static inline bool loadedLmr = false;

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

            StackItem stack[MAX_PLY + 10];
            StackItem* root = stack + 5;

            for (int i = 0; i < MAX_PLY - 1; i++){
                (root + i)->ply = i;
            }

            reset_killers(root->ply);

            int16_t eval;
            for (int depth = 1; depth <= searchDepth; depth++){
                (root + 1)->cutoffCount = 0;
                if (depth <= 5){
                    eval = negamax<ROOT>(depth, MIN_VALUE, MAX_VALUE, root, false);

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
                    eval = negamax<ROOT>(depth, alpha, beta, root, false);

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
        int16_t negamax(int depth, int16_t alpha, int16_t beta, StackItem* stack, bool cutNode) {
            constexpr bool root_node = nodeType == ROOT;
            constexpr bool pv_node = nodeType != NONPV;

            if constexpr (pv_node)
                result.pvLength[stack->ply] = 0;

            (stack + 2)->cutoffCount = 0;

            if (result.nodesVisited & 2048 && is_time_out())
                return MIN_VALUE;

            if (!root_node && board.is_draw())
                return DRAW;

            if (stack->ply >= MAX_PLY)
                return board.eval();

            const bool is_singular = stack->excludedMove != Move::none();

            auto [entry, tt_hit] = tt->probe(board.key());
            const bool tt_capture = tt_hit && board.is_capture(entry.move);

            if (!pv_node && tt_hit && entry.depth >= depth && !is_singular){

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

            int16_t alpha_md = std::max(int(alpha), -CHECKMATE + stack->ply);
            int16_t beta_md = std::min(int(beta), CHECKMATE - stack->ply - 1);
            if(!root_node && alpha_md >= beta_md)
                return alpha_md;

            stack->can_null = (stack - 1)->can_null;
            const int16_t static_eval = stack->eval = board.eval();
            const bool in_check = board.in_check();
            const bool improving = stack->eval > (stack - 2)->eval;

            reset_killers(stack->ply + 1);

            if (!in_check && !is_singular) {

                // IIR.
                if ((pv_node || cutNode) && depth >= 5 && !tt_hit)
                    depth--;

                // Reverse futility pruning.
                // If eval is really good, that even with big margin beats beta, return static eval.
                if (!pv_node && depth <= 8 && static_eval >= beta + 100 * depth - improving * 50)
                    return static_eval;

                // Null move pruning.
                const bool some_piece = board.some_big_piece();
                if (!pv_node && depth >= 3 && stack->can_null && some_piece
                    && static_eval >= beta){

                    int16_t reduction = 3 + depth / 3;
                    int16_t nmp_depth = std::max(depth - reduction, 1);

                    stack->can_null = false;
                    stack->currentMove = Move::null();
                    stack->movedPiece = NONE;

                    board.make_null_move();
                    const int16_t value = -negamax<NONPV>(nmp_depth, -beta, -beta + 1, stack + 1, !cutNode);
                    board.undo_null_move();

                    stack->can_null = true;

                    if (value >= beta)
                        return value;
                }

                // Razoring.
                if (!pv_node && depth <= 4 && std::abs(alpha) < CHECKMATE_BOUND &&
                    static_eval + 300 + 250 * depth <= alpha){

                    const int16_t value = q_search(alpha, alpha + 1, stack);
                    if (value <= alpha)
                        return value;
                }
            }

            MoveList<false> ml(&board, &mainHistory, &entry.move, &continuationHistory,
                               stack, &captureHistory, &killerMoves[stack->ply]);

            Move move;
            int move_count = 0;
            Move best_move = Move::none();
            int16_t best_value = MIN_VALUE;
            std::vector<Move> quiet_moves;
            std::vector<Move> capture_moves;

            TTFlag flag = UPPER_BOUND;

            while ((move = ml.get()) != Move::none()){

                if (move == stack->excludedMove)
                    continue;

                const bool is_capture = board.is_capture(move);
                stack->movedPiece = board.at(move.from());
                stack->currentMove = move;

                if (!is_capture && !in_check){

                    // Futility pruning.
                    // If current eval is bad, we don't expect that quiet move will help us in this position,
                    // so we can skip it.
                    if (!pv_node && move_count && static_eval + 110 * depth <= alpha && depth <= 8 && !is_singular)
                        continue;

                    // Late move pruning.
                    const int move_count_limit = 3 + (depth * depth) / (2 - improving);
                    if (!pv_node && move_count > move_count_limit && !move.is_promotion())
                        continue;

                    // SEE pruning of quiets.
                    if(!root_node && depth <= 7 && alpha > -CHECKMATE_BOUND && !board.see(move, -80 * depth))
                        continue;
                }

                // SEE pruning of captures.
                if (!root_node && depth <= 7 && is_capture && !in_check && !board.see(move, -40 * depth * depth))
                    continue;

                // Singular extensions.
                int extension = 0;
                if (!root_node && move == entry.move &&
                    depth >= 8 && entry.flag != UPPER_BOUND
                    && entry.depth + 3 >= depth && std::abs(entry.eval) < CHECKMATE_BOUND){

                    const int16_t singular_beta = entry.eval - depth;
                    const int singular_depth = (depth - 1) / 2;

                    stack->excludedMove = move;
                    const int16_t value = negamax<NONPV>(singular_depth, singular_beta - 1, singular_beta, stack, cutNode);
                    stack->excludedMove = Move::none();

                    if (value < singular_beta)
                        extension = 1 + (!pv_node && value + 25 < singular_beta);

                    else if (entry.eval >= beta)
                        extension = -1;
                    else if (cutNode)
                        extension = -2;
                }

                int move_score = 0;
                if (is_capture){
                    int to_square = move.to();
                    Piece captured_piece = move.special_type() == Move::EN_PASSANT ? PAWN : board.at(move.to());
                    move_score = captureHistory[stack->movedPiece][to_square][captured_piece];
                }
                else{
                    move_score = mainHistory[board.whoPlay][move.from()][move.to()];
                }

                if (!board.make_move(move))
                    continue;

                result.nodesVisited++;
                move_count++;
                tt->prefetch(board.key());

                int16_t value;
                int16_t reduction = 0;

                const int new_depth = depth - 1 + extension;
                if (depth >= 3 && !root_node && move_count > 3){

                    reduction = lmrTable[depth - 1][move_count - 1];
                    if (pv_node)
                        reduction -= 128;

                    if (!is_capture && tt_capture)
                        reduction += 64;

                    if (!improving)
                        reduction += 64;

                    if (cutNode)
                        reduction += 128;

                    if (board.in_check())
                        reduction -= 128;

                    if ((stack + 1)->cutoffCount > 10)
                        reduction += 128;

                    reduction -= move_score / 256;

                    reduction /= 128; // Scaling to a depth.
                    reduction = std::clamp((int)reduction, 0, new_depth - 2);

                    value = -negamax<NONPV>(new_depth - reduction, -alpha - 1, -alpha, stack + 1, true);

                    if (value > alpha && reduction)
                        value = -negamax<NONPV>(new_depth, -alpha - 1, -alpha, stack + 1, !cutNode);
                }

                else if (!pv_node || move_count > 1)
                    value = -negamax<NONPV>(new_depth, -alpha - 1, -alpha, stack + 1, !cutNode);

                if (pv_node && (move_count == 1 || value > alpha))
                    value = -negamax<PV>(new_depth, -beta, -alpha, stack + 1, false);

                board.undo_move();

                if (is_time_out())
                    return MIN_VALUE;

                if (value > best_value) {
                    best_value = value;

                    if constexpr (root_node){
                        result.bestMove = move;
                        update_pv<true>(stack->ply, move);
                    }

                    if (value > alpha){
                        best_move = move;
                        alpha = value;
                        flag = EXACT;

                        update_pv<pv_node>(stack->ply, move);
                    }

                    if (value >= beta){
                        flag = LOWER_BOUND;

                        assert(best_move == move);
                        if (!is_capture) {
                            update_continuation_histories(stack, best_move, quiet_moves, depth);
                            update_main_history(best_move, quiet_moves, depth);
                            store_killer_move(stack->ply, best_move);
                        }
                        else{
                            update_capture_history(best_move, capture_moves, depth);
                        }

                        stack->cutoffCount++;
                        break;
                    }
                }

                if (!is_capture && move != best_move)
                    quiet_moves.emplace_back(move);

                if (is_capture && move != best_move)
                    capture_moves.emplace_back(move);
            }

            if (move_count == 0 && in_check)
                return -CHECKMATE + stack->ply;
            else if (move_count == 0)
                return DRAW;

            if (!is_singular)
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

            if (result.nodesVisited & 2048 && is_time_out())
                return MIN_VALUE;

            MoveList<true> ml(&board);
            Move move;

            const bool in_check = board.in_check();
            while ((move = ml.get()) != Move::none()){
                
                if (!in_check && !board.see(move, 0))
                    continue;

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


        template<bool pv_node>
        void update_pv(const int ply, const Move& move){
            if constexpr (pv_node){
                result.pvTable[ply][0] = move;
                for (int i = 0; i < result.pvLength[ply + 1]; i++) {
                    result.pvTable[ply][i + 1] = result.pvTable[ply + 1][i];
                }
                result.pvLength[ply] = result.pvLength[ply + 1] + 1;
            }
        }

        void update_main_history(const Move& bestMove, const std::vector<Move>& quietMoves, const int depth){
            int bonus = std::min(150 * depth, 1650);
            apply_gravity(mainHistory[board.whoPlay][bestMove.from()][bestMove.to()], bonus, MainHistory::maxValue);

            for (const Move& move: quietMoves)
                apply_gravity(mainHistory[board.whoPlay][move.from()][move.to()], -bonus, MainHistory::maxValue);
        }


        void update_continuation_histories(const StackItem* stack,
                                           const Move& bestMove,
                                           const std::vector<Move>& quietMoves,
                                           const int depth){

            int bonus = std::min(110 * depth, 1650);
            update_continuation_histories_move(stack, bestMove, bonus, board.at(bestMove.from()));

            for (const Move& move : quietMoves)
                update_continuation_histories_move(stack, move, -bonus, board.at(move.from()));
        }

        void update_continuation_histories_move(const StackItem* stack, const Move& move, int bonus, const Piece movedPiece){
            assert(movedPiece != NONE);

            for (int n_ply = 1; n_ply <= CONT_HIST_MAX_PLY; n_ply++){
                const Move& previous_move = (stack - n_ply)->currentMove;
                const Piece previous_piece = (stack - n_ply)->movedPiece;
                if (previous_move == Move::none() || previous_move == Move::null())
                    break;

                int& entry = continuationHistory[n_ply - 1][previous_piece][previous_move.to()][movedPiece][move.to()];
                apply_gravity(entry, bonus, ContinuationHistoryEntry::maxValue);
            }
        }

        void update_capture_history(const Move& bestMove,
                                    const std::vector<Move>& captureMoves,
                                    const int depth){

            int bonus = std::min(150 * depth, 1650);

            auto update_capture_history = [this](const Move& move, const int bonus){
                Piece moved_piece = board.at(move.from());
                int to_square = move.to();
                Piece captured_piece = move.special_type() == Move::EN_PASSANT ? PAWN : board.at(move.to());
                assert(captured_piece != NONE);
                apply_gravity(captureHistory[moved_piece][to_square][captured_piece], bonus, CaptureHistory::maxValue);
            };

            update_capture_history(bestMove, bonus);

            for (const Move& move: captureMoves) {
                update_capture_history(move, -bonus);
            }
        }

        void store_killer_move(int ply, const Move& move){
            killerMoves[ply][1] = killerMoves[ply][0];
            killerMoves[ply][0] = move;
        }

        void reset_killers(int ply){
            killerMoves[ply][0] = killerMoves[ply][1] = Move::none();
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

            for (auto& pc: captureHistory)
                for (auto& square: pc)
                    for (auto& pc2 : square)
                        pc2 = 0;

            if (loadedLmr)
                return;

            for (int depth = 1; depth <= MAX_PLY; depth++)
                for (int mc = 1; mc <= MAX_POSSIBLE_MOVES; mc++)
                    lmrTable[depth - 1][mc - 1] = int16_t((0.75 + log(depth) * log(mc) * 0.35) * 128);

            loadedLmr = true;
        }
    };
}

#endif //SIGMOID_WORKER_HPP
