#include <cstdint>
#include <cstring>

#include "move.hpp"

#ifndef SIGMOID_TT_HPP
#define SIGMOID_TT_HPP

namespace Sigmoid {
    __extension__ typedef unsigned __int128 uint128_t;

    enum TTFlag : int8_t {
        LOWER_BOUND = 1,
        UPPER_BOUND = 2,
        EXACT = 3
    };

    struct Entry {
        uint32_t key = 0;
        Move move = Move::none();
        TTFlag flag;
        int8_t depth = 0;
        int16_t eval = 0;
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

        inline uint32_t entry_key(uint64_t key){
            return uint32_t(key >> 32);
        }

        void store(uint64_t key, const Move& move, TTFlag flag, int8_t depth, int16_t eval, int16_t ply){
            if (eval >= CHECKMATE_BOUND) eval += ply;
            else if (eval <= -CHECKMATE_BOUND) eval -= ply;
            const int index = get_index(key);
            Entry& entry = entries[index];

            uint32_t e_key = entry_key(key);

            if (entry.key != e_key || depth > entry.depth || flag == EXACT)
                entry = {e_key, move, flag, depth, eval};
        }

        void prefetch(uint64_t key){
            __builtin_prefetch(&entries[get_index(key)]);
        }

        std::pair<Entry, bool> probe(uint64_t key){
            const int index = get_index(key);
            Entry entry = entries[index];

            const bool tt_hit = entries[index].key == entry_key(key);
            entry.move = tt_hit ? entry.move : Move::none();
            return {entry, tt_hit};
        }


        inline int get_index(const uint64_t& key){
            return int(((uint128_t)(key) * (uint128_t)(numberOfEntries)) >> 64);
        }

        ~TranspositionTable(){
            delete[] entries;
        }
    };
}

#endif //SIGMOID_TT_HPP
