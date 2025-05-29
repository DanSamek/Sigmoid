#ifndef SIGMOID_ENGINE_HPP
#define SIGMOID_ENGINE_HPP

#include "move.hpp"
#include "board.hpp"

namespace Sigmoid {

    struct Engine {
        struct Options {
            int depth;
            int wTime, bTime;
            int wInc, bInc;

            int threadCnt;
            // int ttSizeMb; TODO TTStruct here.
            Board board;
        };

        void start_searching(const Options& options){
            Board cpy = options.board;
            MoveList<false> ml(&options.board);
            Move move;
            while ((move = ml.get()) != Move::none()){
                if (!cpy.make_move(move))
                    continue;

                break;
            }
            std::cout << "bestmove " << move.to_uci() << std::endl;
        }

        void stop_searching(){ }
    };
}

#endif //SIGMOID_ENGINE_HPP
