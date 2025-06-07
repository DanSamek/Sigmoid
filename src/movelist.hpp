#ifndef SIGMOID_MOVELIST_HPP
#define SIGMOID_MOVELIST_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"
#include "movegen.hpp"
#include "history.hpp"

namespace Sigmoid {

    template<bool captures>
    struct MoveList {
        MoveList(const Board* board, const MainHistory* mainHistory) : board(board), mainHistory(mainHistory){}
        MoveList(const Board* board) : board(board){}


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
                Move move = moves[i];
                bool capture = board->is_capture(move);

                Piece from = board->at(move.from());
                if (capture){
                    Piece to = move.special_type() == Move::EN_PASSANT ? PAWN : board->at(move.to());
                    scores[i] = ((to + 1) * 1000) * (KING - from + 1) + QUIET_OFFSET;
                }
                else{
                    if (mainHistory)
                        scores[i] = (*mainHistory)[board->whoPlay][move.from()][move.to()];
                }
            }
        }

        const Board* board;
        const MainHistory* mainHistory = nullptr;

        std::array<Move, MAX_POSSIBLE_MOVES> moves;

        std::array<int, MAX_POSSIBLE_MOVES> scores;
        int size = 0;

        bool generated = false;
    };
}

#endif //SIGMOID_MOVELIST_HPP
