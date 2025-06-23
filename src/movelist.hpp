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
                 const StackItem* stack,
                 const CaptureHistory::type* captureHistory,
                 const std::array<Move, 2>* killerMoves)
                 : board(board), mainHistory(mainHistory), ttMove(ttMove),
                   continuationHistory(continuationHistory), stack(stack),
                   captureHistory(captureHistory), killerMoves(killerMoves){}

        MoveList(const Board* board) : board(board) {}

        MoveList(const Board* board,
                 const Move* ttMove)
                 : board(board), ttMove(ttMove) {}


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

            int best_move_idx = 0;
            for (int i = 1; i < size; ++i)
                if (scores[i] > scores[best_move_idx])
                    best_move_idx = i;

            Move best_move = moves[best_move_idx];
            scores[best_move_idx] = scores[size - 1];
            moves[best_move_idx] = moves[size - 1];
            size--;

            return best_move;
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
                    scores[i] += ((captured_piece + 1) * 100) * (KING - from_piece + 1);

                    // Bad captures.
                    if (!board->see(move, -SEE_VALUES[PAWN]))
                        scores[i] = -1'000'000;

                    else if (captureHistory){
                        Piece moved_piece = board->at(move.from());
                        int to_square = move.to();

                        scores[i] += (*captureHistory)[moved_piece][to_square][captured_piece];
                    }
                }
                else{
                    if (!mainHistory) continue;

                    if (killerMoves[stack->ply][0] == move)
                        scores[i] = KILLER_BONUS_0;

                    else if (killerMoves[stack->ply][1] == move)
                        scores[i] = KILLER_BONUS_1;

                    else {
                        scores[i] = (*mainHistory)[board->whoPlay][move.from()][move.to()];

                        auto get_cont_ply_hist = [&]() -> int {
                            int cont_ply_hist_score = 0;
                            for (int n_ply = 1; n_ply <= CONT_HIST_MAX_PLY; n_ply++) {
                                const Move &previous_move = (stack - n_ply)->currentMove;
                                const Piece previous_moved_piece = (stack - n_ply)->movedPiece;
                                if (previous_move == Move::none() || previous_move == Move::null())
                                    break;

                                const Piece current_piece = board->at(move.from());
                                int16_t entry =
                                        (*continuationHistory)[n_ply -1][previous_moved_piece][previous_move.to()][current_piece][move.to()];
                                cont_ply_hist_score += entry;
                            }
                            return cont_ply_hist_score;
                        };
                        scores[i] += get_cont_ply_hist();
                    }
                }
            }
        }

        const Board* board;
        const MainHistory::type* mainHistory = nullptr;
        const Move* ttMove = nullptr;
        const ContinuationHistory* continuationHistory = nullptr;
        const StackItem* stack = nullptr;
        const CaptureHistory::type* captureHistory = nullptr;
        const std::array<Move, 2>* killerMoves = nullptr;

        std::array<Move, MAX_POSSIBLE_MOVES> moves;
        std::array<int, MAX_POSSIBLE_MOVES> scores;
        int size = 0;

        bool generated = false;
    };
}

#endif //SIGMOID_MOVELIST_HPP
