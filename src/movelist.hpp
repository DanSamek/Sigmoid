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
        MoveList(const Board* board,
                 const MainHistory* mainHistory,
                 const Move* ttMove,
                 const KillerMoves* killerMoves,
                 const int ply)
             : board(board), mainHistory(mainHistory), ttMove(ttMove), killerMoves(killerMoves), ply(ply){}

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
                    scores[i] += ((captured_piece + 1) * 10000) * (KING - from_piece + 1);
                }
                else{
                    if (!mainHistory) continue;

                    scores[i] = (*mainHistory)[board->whoPlay][move.from()][move.to()];

                    if (move == (*killerMoves)[ply])
                        scores[i] = KILLER_1_VALUE;

                }
            }
        }

        const Board* board;
        const MainHistory* mainHistory = nullptr;
        const Move* ttMove = nullptr;
        const KillerMoves* killerMoves = nullptr;
        int ply = 0;

        std::array<Move, MAX_POSSIBLE_MOVES> moves;
        std::array<int, MAX_POSSIBLE_MOVES> scores;
        int size = 0;

        bool generated = false;
    };
}

#endif //SIGMOID_MOVELIST_HPP
