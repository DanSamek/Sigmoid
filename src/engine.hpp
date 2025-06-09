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
        std::vector<Worker> workers;

        struct Options {
            int depth = MAX_PLY - 1;
            int64_t wTime = 0, bTime = 0;
            int64_t wInc = 0, bInc = 0;

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
            Timer timer(options.wTime, options.bTime, options.wInc, options.bInc, options.board.whoPlay);
            WorkerHelper worker_helper(workers.size(), options.datagen, &timer);
            std::vector<std::thread> search_threads;

            for (size_t i = 0; i < workers.size(); ++i) {
                workers[i].load_state(options.board, options.tt, &worker_helper, &timer, options.depth);
                search_threads.emplace_back(&Worker::iterative_deepening, &workers[i]);
            }

            for (std::thread& search_thread : search_threads)
                search_thread.join();

            options.score = worker_helper.bestResult.score;
            options.totalNodesVisited = worker_helper.totalNodesVisited;
            if (!options.datagen)
                std::cout << "bestmove " << worker_helper.bestResult.bestMove.to_uci() << std::endl;
        }

        void new_game(int threadCnt){
            workers = std::vector<Worker>(threadCnt);

            for (Worker& worker: workers)
                worker.new_game();
        }
    };
}

#endif //SIGMOID_ENGINE_HPP
