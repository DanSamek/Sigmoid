#ifndef SIGMOID_ENGINE_HPP
#define SIGMOID_ENGINE_HPP

#include <thread>

#include "move.hpp"
#include "board.hpp"
#include "tt.hpp"
#include "thread.hpp"

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
            std::vector<std::thread> search_threads;

            ThreadHelper threadHelper(options.threadCnt, options.datagen);

            for (int i = 0; i < options.threadCnt; ++i) {
                Board board_copy = options.board;

                search_threads.emplace_back([board_copy, &options, &threadHelper] {
                    Thread* th = new Thread(board_copy, options.tt, &threadHelper, options.depth);
                    th->iterative_deepening();
                    delete th;
                });
            }

            for (std::thread& search_thread : search_threads)
                search_thread.join();

            options.score = threadHelper.bestResult.score;
        }
    };
}

#endif //SIGMOID_ENGINE_HPP
