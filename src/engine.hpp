#ifndef SIGMOID_ENGINE_HPP
#define SIGMOID_ENGINE_HPP

#include <thread>
#include <future>

#include "move.hpp"
#include "board.hpp"
#include "tt.hpp"
#include "thread.hpp"

namespace Sigmoid {

    struct Engine {
        struct Options {
            int depth;
            int wTime, bTime;
            int wInc, bInc;

            int threadCnt;
            TranspositionTable* tt;
            Board board;
        };

        void start_searching(const Options& options){
            Board cpy = options.board;
            std::vector<std::thread> search_threads;
            std::vector<std::future<SearchResult>> search_futures;
            std::vector<SearchResult> search_results;

            for (int i = 0; i < options.threadCnt; ++i){
                Thread th(i, cpy, options.tt);
                search_futures.emplace_back(std::async(std::launch::async, [th]() mutable {
                    return th.iterative_deepening();
                }));
            }

            for (std::thread& search_thread : search_threads)
                search_thread.join();

            for (auto& future : search_futures)
                search_results.push_back(future.get());

            // TODO find best.
            /*
                MoveList<false> ml(&options.board);
                Move move;
                while ((move = ml.get()) != Move::none()){
                    if (!cpy.make_move(move))
                        continue;
                    break;
                }
                std::cout << "bestmove " << move.to_uci() << std::endl;
            */
        }

        void stop_searching(){ }
    };
}

#endif //SIGMOID_ENGINE_HPP
