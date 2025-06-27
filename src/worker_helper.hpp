#ifndef SIGMOID_WORKER_HELPER_HPP
#define SIGMOID_WORKER_HELPER_HPP

#include <mutex>
#include <vector>
#include <iostream>

#include "search.hpp"
#include "constants.hpp"
#include "timer.hpp"

namespace Sigmoid{

    struct WorkerHelper{
        const size_t threadCnt;
        std::map<int, std::vector<SearchResult>> depthSearchDone;
        std::mutex resultLock;
        SearchResult bestResult;
        bool datagen;
        uint64_t totalNodesVisited = 0ULL;
        Timer* timer;

        WorkerHelper(int threadCnt, bool datagen, Timer* timer) : threadCnt(threadCnt), datagen(datagen), timer(timer) { }

        void enter_search_result(const int searchDepth, const SearchResult& searchResult){
            std::unique_lock lock(resultLock);
                depthSearchDone[searchDepth].emplace_back(searchResult);

                if (depthSearchDone[searchDepth].size() == threadCnt)
                    save_depth_best_result(searchDepth);
        }

        void save_depth_best_result(const int searchDepth){
            std::map<Move, int> votes;

            for (const SearchResult& s_result : depthSearchDone[searchDepth]) {
                totalNodesVisited += s_result.nodesVisited;
                votes[s_result.bestMove] ++;
            }

            Move voted_move = Move::none();
            int voted_move_votes = 0;
            for (const auto& [move, total_votes] : votes){
                if (total_votes <= voted_move_votes) continue;

                voted_move = move;
                voted_move_votes = total_votes;
            }

            for (const SearchResult& s_result: depthSearchDone[searchDepth]){
                if (s_result.bestMove != voted_move) continue;

                bestResult = s_result;
                break;
            }

            depthSearchDone.erase(searchDepth);

            if (datagen) return;

            print_result(searchDepth);
        }

        void print_result(const int searchDepth){
            int64_t ms = timer->get_ms();
            if (!ms)
                ms = 1;

            assert(bestResult.bestMove == bestResult.pvTable[0][0]);

            const bool is_mate = std::abs(bestResult.score) > CHECKMATE_BOUND;
            const bool winning_mate = is_mate && bestResult.score > 0;
            std::cout << "info score ";
            if (!is_mate){
                std::cout << "cp " << bestResult.score;
            }
            else{
                std::cout << "mate ";
                const int ply = std::abs(std::abs(bestResult.score) - CHECKMATE);
                if (!winning_mate)
                    std::cout << "-";
                std::cout << (ply + 1) / 2;
            }

            std::cout << " depth "<< searchDepth;
            std::cout << " nodes " << totalNodesVisited  << " time " << ms << " nps " << (totalNodesVisited * 1000) / ms;

            std::cout << " pv ";
            for (int i = 0; i < bestResult.pvLength[0]; i++)
                std::cout << bestResult.pvTable[0][i].to_uci() << " ";

            std::cout << std::endl;
        }
    };
}

#endif //SIGMOID_WORKER_HELPER_HPP
