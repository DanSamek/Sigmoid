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

        struct Options{
            int depth = MAX_PLY - 1;
            int64_t wTime = 0, bTime = 0;
            int64_t wInc = 0, bInc = 0;

            TranspositionTable* tt = nullptr;
            Board board;

            // Datagen stuff
            int softNodes = 5000;
            bool datagen = false;

            // Out values.
            int64_t totalNodesVisited;
        };

        void start_searching(Options& options){
            Timer timer(options.wTime, options.bTime, options.wInc, options.bInc, options.board.whoPlay);
            WorkerHelper worker_helper(workers.size(), options.datagen, &timer);
            std::vector<std::thread> search_threads;
            std::vector<Board*> boards;
            std::vector<SearchResult*> search_results;

            for (size_t i = 0; i < workers.size(); ++i) {
                boards.emplace_back(new Board(options.board));
                search_results.emplace_back(new SearchResult());
                workers[i].load_state(boards[i], options.tt,
                                      &worker_helper, &timer,
                                      options.depth, options.datagen,
                                      options.softNodes, search_results[i]);

                search_threads.emplace_back(&Worker::iterative_deepening, &workers[i]);
            }

            for (std::thread& search_thread : search_threads)
                search_thread.join();

            for (Board* board : boards)
                delete board;

            for (SearchResult* sr : search_results)
                delete sr;

            options.totalNodesVisited = worker_helper.totalNodesVisited;
            std::cout << "bestmove " << worker_helper.bestResult.bestMove.to_uci() << std::endl;
        }


        struct DatagenOptions{
            // Inputs.
            TranspositionTable* tt = nullptr;
            Board* board;
            int softNodes = 5000;
        };

        void datagen(DatagenOptions& options, SearchResult* searchResult){
            workers[0].load_state(options.board, options.tt, nullptr, nullptr, MAX_PLY - 1, true, options.softNodes, searchResult);
            workers[0].iterative_deepening();
        }

        void new_game(int threadCnt){
            workers = std::vector<Worker>(threadCnt);

            for (Worker& worker: workers)
                worker.new_game();
        }
    };
}

#endif //SIGMOID_ENGINE_HPP
