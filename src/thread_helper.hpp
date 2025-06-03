#ifndef SIGMOID_THREAD_HELPER_HPP
#define SIGMOID_THREAD_HELPER_HPP

#include <mutex>
#include <vector>

#include "search.hpp"
#include "constants.hpp"

namespace Sigmoid{

    struct ThreadHelper{
        const size_t threadCnt;
        std::map<int, std::vector<SearchResult>> depthSearchDone;
        std::mutex resultLock;
        SearchResult bestResult;
        bool datagen;

        ThreadHelper(int threadCnt, bool datagen) : threadCnt(threadCnt), datagen(datagen) { }

        void enter_search_result(const int searchDepth, const SearchResult& searchResult){
            std::unique_lock lock(resultLock);
                depthSearchDone[searchDepth].emplace_back(searchResult);

                if (depthSearchDone[searchDepth].size() == threadCnt)
                    save_depth_best_result(searchDepth);
        }

        void save_depth_best_result(const int searchDepth){
            std::map<Move, int> votes;

            uint64_t total_nodes_visited = 0;
            for (const SearchResult& s_result : depthSearchDone[searchDepth]) {
                total_nodes_visited += s_result.nodesVisited;
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
            std::cout << "info score cp " << bestResult.score << " depth "
            << searchDepth << " bestmove " << bestResult.bestMove.to_uci()
            << " nodes "<< total_nodes_visited << std::endl;
        }
    };
}

#endif //SIGMOID_THREAD_HELPER_HPP
