#ifndef SIGMOID_MAGICS_HPP
#define SIGMOID_MAGICS_HPP

#include <cstdint>
#include <array>
#include <vector>
#include <cassert>
#include <math.h>

#include "helper.hpp"
#include "bitops.hpp"

namespace Sigmoid {

    // Same implementation as in Sentinel.
    // https://github.com/DanSamek/Sentinel/blob/main/src/magics.h
    // https://github.com/DanSamek/Sentinel/blob/main/src/magics.cpp
    struct Magics {
        static inline bool ready = false;

        // Blocked masks for each square.
        static inline std::array<uint64_t, 64> ROOK_BLOCKERS;
        static inline std::array<uint64_t, 64> BISHOP_BLOCKERS;


        static inline std::vector<uint64_t> ROOK_TABLE[64];
        static inline std::vector<uint64_t> BISHOP_TABLE[64];


        static inline std::vector<std::pair<int, int>> rookDirections = {{0,  1},
                                                                         {1,  0},
                                                                         {-1, 0},
                                                                         {0,  -1}};
        static inline std::vector<std::pair<int, int>> bishopDirections = {{1,  1},
                                                                           {-1, -1},
                                                                           {1,  -1},
                                                                           {-1, 1}};

        // generated Magics for rooks
        static inline std::array<uint64_t, 64> ROOK_MAGICS = {
                0x2480102040008002, 0x40025000600240, 0x8200082200418013, 0x830004a100289000, 0x200040200200810,
                0x200080410052200, 0x1000900040d8200, 0x1280004538800100,
                0x4004800284204000, 0x1400400020005000, 0x10071006000c0, 0x242001822001140, 0x1e0800400380080,
                0x244c800c00020080, 0x4500010082000c, 0x8842002c00804a01,
                0x90a060022810040, 0x810004000201040, 0x4110040200100, 0x818048001000, 0x5010004101801,
                0x404008002008004, 0x800808001002200, 0x80080200290080c4,
                0x8812380014002, 0x920006140100142, 0x2200103001040a0, 0x402180180100280, 0x4000480280080,
                0x4015020080040080, 0x200110080800200, 0x2a10204200008104,
                0x903884002800020, 0x20100040c00620, 0x2d02014012002080, 0x901001010028a0, 0x20080185001100,
                0x850808600804400, 0x8010800200800100, 0x5016200008c,
                0x80004020024000, 0x400181010022, 0x206002c111010020, 0x20020051420a0020, 0x4000800110100,
                0x2002010410420008, 0x20410802040010, 0x100004085020024,
                0x4046a0400f800080, 0x504400020018180, 0x210200100904100, 0x8100801000080480, 0x8001005008004d00,
                0x81a01084c100200, 0x44280142100c00, 0x804104088a00,
                0x1094214080015301, 0x130840028021, 0x80208200081042, 0x11605000180501, 0xc001000410080083,
                0x2004801108422, 0x400011040880204, 0x8028410214302
        };

        static inline std::array<uint64_t, 64> ROOK_MAGICS_SHIFT = {
                12, 11, 11, 11, 11, 11, 11, 12,
                11, 10, 10, 10, 10, 10, 10, 11,
                11, 10, 10, 10, 10, 10, 10, 11,
                11, 10, 10, 10, 10, 10, 10, 11,
                11, 10, 10, 10, 10, 10, 10, 11,
                11, 10, 10, 10, 10, 10, 10, 11,
                11, 10, 10, 10, 10, 10, 10, 11,
                12, 11, 11, 11, 11, 11, 11, 12
        };

        // generated Magics for bishops
        static inline std::array<uint64_t, 64> BISHOP_MAGICS = {
                0x4010048104102200, 0x285100c0044808a, 0x800810e0005c0, 0x20920040000100, 0x204142064402084,
                0x1862020070010, 0x80201132010c800, 0xb1c9040654040400,
                0x7000100248083080, 0x2080241802005a00, 0x80124820d4008002, 0x400044100200001, 0x8182308820002411,
                0x209220804440074, 0xa0010806100500, 0x200020241182860,
                0x320606b161c0800, 0x2142002220208, 0x8002001000320020, 0x88002404200c10, 0x14000080a02082,
                0xa02c00201100500, 0x4444000100c81400, 0x101040202410400,
                0x48c140810105008, 0x412220001004008a, 0x3248080001014109, 0xa420202008018060, 0x2001840002802000,
                0x42408004100400, 0x80808902048c1081, 0x8022020200210108,
                0x14020a400090100c, 0x10c4240101002, 0x8009004100080808, 0x1808200800010810, 0x20048400008020,
                0x520088980210800, 0x5011204010840, 0x80805ad00202100,
                0xa0202200c032000, 0x41031010288246, 0x401004022001000, 0x1402a028008420, 0x8201080102430400,
                0x4401101000404080, 0x350a120842040300, 0x9210008200800840,
                0x1801010120208200, 0x3c010d480c242a10, 0x8240442108085010, 0x540000840420ca, 0x6801006120010,
                0x14021020114100a0, 0x34400418448d0c22, 0x60022202002188,
                0x240200e402221080, 0x4002444212032082, 0xc003000100491008, 0x2020602420880, 0x1489a04050400,
                0x40000810018208, 0x20a410022040, 0x124291004008485,
        };

        static inline std::array<uint64_t, 64> BISHOP_MAGICS_SHIFT = {
                6, 5, 5, 5, 5, 5, 5, 6,
                5, 5, 5, 5, 5, 5, 5, 5,
                5, 5, 7, 7, 7, 7, 5, 5,
                5, 5, 7, 9, 9, 7, 5, 5,
                5, 5, 7, 9, 9, 7, 5, 5,
                5, 5, 7, 7, 7, 7, 5, 5,
                5, 5, 5, 5, 5, 5, 5, 5,
                6, 5, 5, 5, 5, 5, 5, 6
        };


        inline static uint64_t get_rook_moves(const uint64_t &blockers, int square) {
            uint64_t hashBlockers = blockers & ROOK_BLOCKERS[square];
            uint64_t hash = hashBlockers * ROOK_MAGICS[square];
            uint64_t index = (hash >> (64ULL - ROOK_MAGICS_SHIFT[square]));
            return ROOK_TABLE[square][index];
        }

        inline static uint64_t get_bishop_moves(const uint64_t &blockers, int square) {
            uint64_t hashBlockers = blockers & BISHOP_BLOCKERS[square];
            uint64_t hash = hashBlockers * BISHOP_MAGICS[square];
            uint64_t index = (hash >> (64ULL - BISHOP_MAGICS_SHIFT[square]));
            return BISHOP_TABLE[square][index];
        }

        static void init_magics() {
            for (int rank = 0; rank < 8; rank++) {
                for (int file = 0; file < 8; file++) {
                    int square = get_square(rank, file);
                    ROOK_TABLE[square] = get_magics(file, rank, ROOK_BLOCKERS, true, ROOK_MAGICS[square]);
                    assert(ROOK_TABLE[square].size() != 0);

                    BISHOP_TABLE[square] = get_magics(file, rank, BISHOP_BLOCKERS, false, BISHOP_MAGICS[square]);
                    assert(BISHOP_TABLE[square].size() != 0);
                }
            }
        }

        static void generate_rook_blockers() {
            for (int rank = 0; rank < 8; rank++) {
                for (int file = 0; file < 8; file++) {
                    int square = get_square(rank, file);
                    uint64_t tmp = 0ULL;
                    auto moves = generate_moves_for_directions(rookDirections, rank, file);

                    for (auto move: moves) {
                        set_nth_bit(tmp, move.first * 8 + move.second);
                    }
                    ROOK_BLOCKERS[square] = tmp;
                }
            }
        }

        static void generate_bishop_blockers() {
            for (int rank = 0; rank < 8; rank++) {
                for (int file = 0; file < 8; file++) {
                    uint64_t tmp = 0ULL;
                    int square = get_square(rank, file);
                    auto moves = generate_moves_for_directions(bishopDirections, rank, file);
                    for (auto move: moves) {
                        set_nth_bit(tmp, move.first * 8 + move.second);
                    }
                    BISHOP_BLOCKERS[square] = tmp;
                }
            }
        }

        static std::vector<std::pair<int, int>> generate_moves_for_directions(const std::vector<std::pair<int, int>> &directions,
                                                                       int rank,
                                                                       int file) {
            std::vector<std::pair<int, int>> result;

            for (const auto& direction: directions) {
                int rank_tmp = rank + direction.second;
                int file_tmp = file + direction.first;

                bool added = false;
                while (rank_tmp >= 0 && rank_tmp <= 7 && file_tmp >= 0 && file_tmp <= 7) {
                    result.emplace_back(rank_tmp, file_tmp);
                    rank_tmp += direction.second;
                    file_tmp += direction.first;
                    added = true;
                }

                if (added)
                    result.pop_back();
            }
            return result;
        }

        static void init() {
            if (ready) return;

            generate_bishop_blockers();
            generate_rook_blockers();
            init_magics();
            ready = true;
        }

        static std::vector<uint64_t> get_magics(int file,
                                         int rank,
                                         std::array<uint64_t, 64>& sliderBlockers,
                                         bool rook,
                                         uint64_t magic) {

            int square = get_square(rank, file);
            std::vector<uint64_t> all_blockers = generate_all_blockers_combinations(sliderBlockers[square]);
            std::vector<uint64_t> table = build_table(sliderBlockers[square], file, rank, rook, magic, all_blockers);
            return table;
        }


        static std::vector<uint64_t> build_table(uint64_t blockerBitBoard,
                                          int file,
                                          int rank,
                                          bool rook,
                                          uint64_t magic,
                                          const std::vector<uint64_t> &allBlockers) {

            int indexBits = rook ? ROOK_MAGICS_SHIFT[get_square(rank, file)] : BISHOP_MAGICS_SHIFT[get_square(rank, file)];
            std::vector<uint64_t> table(1 << indexBits, 0);

            for (const auto& blocker: allBlockers) {
                auto moves = generate_slider_moves(file, rank, blocker, rook ? rookDirections : bishopDirections);
                uint64_t index = magic_index(blocker, blockerBitBoard, indexBits, magic);
                uint64_t item = table[index];

                if (item == 0ULL)
                    table[index] = moves;
                else if (moves != item)
                    return {};
            }
            return table;
        }


        static std::vector<uint64_t> generate_all_blockers_combinations(uint64_t bitboard) {
            std::vector<uint64_t> result;
            std::vector<int> bits;

            int move = 0;
            while (move <= 63) {
                if ((bitboard >> move) & 1)
                    bits.push_back(move);
                move++;
            }

            std::size_t n = bits.size();
            int number_of_combinations = std::pow(2, n);

            for (int i = 0; i < number_of_combinations; ++i) {
                uint64_t b = 0ULL;
                for (std::size_t j = 0; j < n; ++j) {
                    if (i & (1 << j)) {
                        set_nth_bit(b, bits[j]);
                    }
                }
                result.push_back(b);
            }

            assert(result.size() == (std::size_t(1 << bits.size())));
            return result;
        }

        static uint64_t magic_index(uint64_t currentBlockers, uint64_t tableBlocker, int indexBits, uint64_t magics){
            uint64_t blockers = currentBlockers & tableBlocker;
            uint64_t hash = blockers * magics;
            uint64_t index = (hash >> (uint64_t)(64 - indexBits));
            return index;
        }

        static uint64_t generate_slider_moves(int file,
                                       int rank,
                                       const uint64_t& bitboard,
                                       const std::vector<std::pair<int, int>>& movement){

            auto board = generate_board_from_bitboard(bitboard);
            std::vector<int> bits;

            for(const auto& direction : movement) {
                int rank_tmp = rank + direction.second;
                int file_tmp = file + direction.first;

                while(rank_tmp >= 0 && rank_tmp <= 7 && file_tmp >= 0 && file_tmp <= 7){
                    int square = rank_tmp * 8 + file_tmp;
                    bits.push_back(square);
                    if(board[rank_tmp][file_tmp])
                        break;

                    rank_tmp += direction.second;
                    file_tmp += direction.first;
                }
            }

            uint64_t result = 0ULL;
            for(auto item: bits) set_nth_bit(result, item);

            return result;
        }

        static std::array<std::array<bool, 8>, 8> generate_board_from_bitboard(const uint64_t& bb) {
            std::array<std::array<bool, 8>, 8> result;

            for(int rank = 0; rank < 8; rank++ ) {
                for (int file = 0; file < 8; file++) {
                    int square = rank * 8 + file;
                    result[rank][file] = get_nth_bit(bb, square);
                }
            }
            return result;
        }
    };
}

#endif //SIGMOID_MAGICS_HPP
