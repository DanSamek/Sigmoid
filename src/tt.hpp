#include <cstdint>
#include <cstring>

#include "move.hpp"

#ifndef SIGMOID_TT_HPP
#define SIGMOID_TT_HPP

namespace Sigmoid {
    enum TTFlag : int8_t {
        LOWER_BOUND = 1,
        UPPER_BOUND = 2,
        EXACT = 3
    };

    struct Entry {
        uint64_t key;
        Move move = Move::none();
        TTFlag flag;
        int8_t depth = 0;
        int16_t eval = 0;
        bool inPv    = false;
    };

    struct TranspositionTable {
        size_t numberOfEntries;
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

        void store(uint64_t key, const Move& move, TTFlag flag, int8_t depth, int16_t eval, int16_t ply, bool inPv){
            if (eval >= CHECKMATE_BOUND) eval += ply;
            else if (eval <= -CHECKMATE_BOUND) eval -= ply;
            const int index = get_index(key);
            Entry& entry = entries[index];

            if (entry.key != key || depth > entry.depth || flag == EXACT)
                entry = {key, move, flag, depth, eval, inPv};
        }

        void prefetch(uint64_t key){
            __builtin_prefetch(&entries[get_index(key)]);
        }

        std::pair<Entry, bool> probe(uint64_t key){
            const int index = get_index(key);
            Entry entry = entries[index];

            const bool tt_hit = entries[index].key == key;
            entry.move = tt_hit ? entry.move : Move::none();
            return {entry, tt_hit};
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
