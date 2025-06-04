#ifndef SIGMOID_ENGINE_HPP
#define SIGMOID_ENGINE_HPP

#include <thread>

#include "move.hpp"
#include "board.hpp"
#include "tt.hpp"
#include "worker.hpp"

namespace Sigmoid {

    struct Engine {
        struct Options {
            int depth = MAX_PLY - 1;
            int wTime, bTime;
            int wInc, bInc;

            int threadCnt;
            TranspositionTable* tt;
            Board board;

            // Datagen stuff
            int softNodes = 5000;
            bool datagen = false;

            int16_t score; // Out value.
        };

        void start_searching(Options& options){
            WorkerHelper worker_helper(options.threadCnt, options.datagen);
            std::vector<std::thread> search_threads;
            std::vector<Worker> workers;

            for (int i = 0; i < options.threadCnt; ++i) {
                workers.emplace_back(options.board, options.tt, &worker_helper, options.depth);
                search_threads.emplace_back(&Worker::iterative_deepening, &workers[i]);
            }

            for (std::thread& search_thread : search_threads)
                search_thread.join();

            options.score = worker_helper.bestResult.score;
        }
    };
}

#endif //SIGMOID_ENGINE_HPP
