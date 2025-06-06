#ifndef SIGMOID_ENGINE_HPP
#define SIGMOID_ENGINE_HPP

#include <thread>

#include "move.hpp"
#include "board.hpp"
#include "tt.hpp"
#include "worker.hpp"
#include "timer.hpp"

namespace Sigmoid {

    struct Engine {
        struct Options {
            int depth = MAX_PLY - 1;
            int wTime, bTime;
            int wInc, bInc;

            int threadCnt = 1;
            TranspositionTable* tt = nullptr;
            Board board;

            // Datagen stuff
            int softNodes = 5000;
            bool datagen = false;

            // Out values.
            int16_t score;
            uint64_t totalNodesVisited;
        };

        void start_searching(Options& options){
            Timer timer(options.wTime, options.wInc, options.bTime, options.bInc, options.board.whoPlay);
            WorkerHelper worker_helper(options.threadCnt, options.datagen, &timer);
            std::vector<std::thread> search_threads;
            std::vector<Worker> workers;

            for (int i = 0; i < options.threadCnt; ++i) {
                workers.emplace_back(options.board, options.tt, &worker_helper, &timer, options.depth);
                search_threads.emplace_back(&Worker::iterative_deepening, &workers[i]);
            }

            for (std::thread& search_thread : search_threads)
                search_thread.join();

            if (options.datagen){
                options.score = worker_helper.bestResult.score;
                options.totalNodesVisited = worker_helper.totalNodesVisited;
            }
            else{
                std::cout << "bestmove " << worker_helper.bestResult.bestMove.to_uci() << std::endl;
            }
        }
    };
}

#endif //SIGMOID_ENGINE_HPP
