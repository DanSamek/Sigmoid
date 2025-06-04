#include <cstdint>
#include <cstring>

#include "move.hpp"

#ifndef SIGMOID_TT_HPP
#define SIGMOID_TT_HPP

namespace Sigmoid {
    enum Flag : int8_t {
        LOWER_BOUND = 1,
        UPPER_BOUND = 2,
        EXACT = 3
    };

    struct Entry {
        uint64_t key;
        Move move = Move::none();
        Flag flag;
        int8_t depth = 0;
        int16_t eval = 0;
    };

    struct TranspositionTable {
        int numberOfEntries;
        Entry* entries = nullptr;

        void resize(int sizeMB) {
            delete[] entries;

            numberOfEntries = (sizeMB * 1024 * 1024) / sizeof(Entry);
            entries = new Entry[numberOfEntries];
            clear();
        }

        void clear(){
            std::fill(entries, entries + numberOfEntries, Entry{});
        }

        void store(uint64_t key, const Move& move, Flag flag, int8_t depth, int16_t eval){
            // TODO handle mates.

            const int index = get_index(key);
            // For now always replace.
            entries[index] = {key, move, flag, depth, eval};
        }

        void prefetch(uint64_t key){
            __builtin_prefetch(&entries[get_index(key)]);
        }

        std::pair<const Entry&, bool> probe(uint64_t key){
            const int index = get_index(key);
            // maybe tt probe copy, cuz multithread.
            const Entry& entry = entries[index];
            return {entry, entry.key == key};
        }

        inline int get_index(const uint64_t& key){
            return int(key % numberOfEntries);
        }

        ~TranspositionTable(){
            delete[] entries;
        }
    };
}

#endif //SIGMOID_TT_HPP
