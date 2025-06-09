#ifndef SIGMOID_MOVELIST_HPP
#define SIGMOID_MOVELIST_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"
#include "movegen.hpp"
#include "history.hpp"
#include "search.hpp"

namespace Sigmoid {

    template<bool captures>
    struct MoveList {

        MoveList(const Board* board,
                 const MainHistory::type* mainHistory,
                 const Move* ttMove,
                 const ContinuationHistory* continuationHistory,
                 const StackItem* stack)
                :  board(board), mainHistory(mainHistory), ttMove(ttMove),
                   continuationHistory(continuationHistory),stack(stack){}

        MoveList(const Board* board) : board(board) {}


        Move get(){
            if (!generated){
                Movegen::generate_moves<captures>(board->currentState, board->whoPlay, moves, size);
                score_moves();
                generated = true;
            }
            return pick_move();
        }

    private:
        Move pick_move(){
            if (size <= 0)
                return Move::none();

            for (int i = 0; i < size - 1; i++){
                if (scores[i] > scores[i + 1]){
                    std::swap(scores[i], scores[i + 1]);
                    std::swap(moves[i], moves[i + 1]);
                }
            }

            return moves[--size];
        }

        void score_moves(){
            for (int i = 0; i < size; i++){
                scores[i] = 0;
                const Move& move = moves[i];
                bool capture = board->is_capture(move);

                if (ttMove && *ttMove == move){
                    scores[i] = TT_MOVE_VALUE;
                }
                else if (capture){
                    Piece captured_piece = move.special_type() == Move::EN_PASSANT ? PAWN : board->at(move.to());
                    Piece from_piece = board->at(move.from());
                    scores[i] = QUIET_OFFSET;
                    scores[i] = ((captured_piece + 1) * 10000) * (KING - from_piece + 1);
                }
                else{
                    if (!mainHistory)
                        continue;

                    scores[i] = (*mainHistory)[board->whoPlay][move.from()][move.to()];
                    auto get_cont_ply_hist = [&]()->int{
                        int cont_ply_hist_score = 0;
                        for (int n_ply = 1; n_ply <= CONT_HIST_MAX_PLY; n_ply++){
                            const Move& previous_move = (stack - n_ply)->currentMove;
                            if (previous_move == NO_MOVE)
                                break;

                            int16_t entry = (*continuationHistory)[n_ply - 1][previous_move.from()][previous_move.to()][move.from()][move.to()];
                            cont_ply_hist_score += entry;
                        }
                        return cont_ply_hist_score;
                    };
                    scores[i] += get_cont_ply_hist();
                }
            }
        }

        const Board* board;
        const MainHistory::type* mainHistory = nullptr;
        const Move* ttMove = nullptr;
        const ContinuationHistory* continuationHistory = nullptr;
        const StackItem* stack = nullptr;

        std::array<Move, MAX_POSSIBLE_MOVES> moves;
        std::array<int, MAX_POSSIBLE_MOVES> scores;
        int size = 0;

        bool generated = false;
    };
}

#endif //SIGMOID_MOVELIST_HPP
