#ifndef SIGMOID_MOVEGEN_HPP
#define SIGMOID_MOVEGEN_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"
#include "board.hpp"
#include "magics.hpp"

namespace Sigmoid{
    // Pseudo-legal movegen.
    struct Movegen{

        template<bool captures>
        void generate_moves(const Board& board, std::array<Move, MAX_POSSIBLE_MOVES>& result_moves){
            // TODO
        }

        void init(){
            Magics::init();
            generate_move_bitboards<KNIGHT>();
            generate_move_bitboards<KING>();

            generate_pawn_bitboards<Color::white()>();
            generate_pawn_bitboards<Color::black()>();
        }


        // Will be removed, when movegen will pass perft tests.
        template<Piece piece>
        void print_bitboards(){
            static_assert(piece == KING || piece == KNIGHT || piece == PAWN);
            if constexpr (piece == PAWN){
                for (int square = 0; square < 64; square++){
                    std::cout << square << std::endl;
                    std::cout << "QUIET" << std::endl;
                    uint64_t bb_w_q = pawnQuietMoves[Color::white()][square];
                    print_bitboard(bb_w_q);
                    std::cout << std::endl;
                    uint64_t bb_b_q = pawnQuietMoves[Color::black()][square];
                    print_bitboard(bb_b_q);
                    std::cout << std::endl;

                    std::cout << "ATTACK" << std::endl;
                    bb_w_q = pawnAttackMoves[Color::white()][square];
                    print_bitboard(bb_w_q);
                    std::cout << std::endl;
                    bb_b_q = pawnAttackMoves[Color::black()][square];
                    print_bitboard(bb_b_q);
                    std::cout << std::endl;
                    std::cout << std::endl;
                }
            }
            else{
                auto& bitboards = piece == KING ? kingMoves : knightMoves;
                for (int square = 0; square < 64; square++){
                    uint64_t bb = bitboards[square];
                    std::cout << square << std::endl;
                    print_bitboard(bb);
                    std::cout << std::endl;
                }
            }
        }

    private:
        static inline std::array<uint64_t, 64> kingMoves, knightMoves;
        static inline std::array<std::array<uint64_t, 64>, 2> pawnQuietMoves, pawnAttackMoves;

        static constexpr std::array<std::array<uint64_t, 2>, 2> CASTLING_FREE_MASKS = {{
            {{0xe00000000000000, 0x6000000000000000}},{{0xe,0x60}}
        }};

        static constexpr std::array<std::pair<int, int>, 8> KNIGHT_MOVES = {{
            {1, 2}, {2, 1}, {-1, 2}, {-2, 1}, {1, -2}, {2, -1}, {-1, -2}, {-2, -1}
        }};

        static constexpr std::array<std::pair<int, int>, 8> KING_MOVES = {{
            {0, 1}, {1, 0}, {1, 1}, {0, -1}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1},
        }};

        static constexpr std::array<std::array<std::pair<int, int>, 1>, 2> PAWN_QUIET_MOVES_1 = {{
            {{ {-1, 0} }}, {{ {1, 0} }}
        }};

        static constexpr std::array<std::array<std::pair<int, int>, 2>, 2> PAWN_QUIET_MOVES_2 = {{
            {{ {-1, 0}, {-2, 0} }}, {{ {1, 0}, {2, 0} }}
        }};

        static constexpr std::array<std::array<std::pair<int, int>, 2>, 2> PAWN_ATTACK_MOVES = {{
            {{ {-1,1}, {-1, -1} }}, {{ {1, 1}, {1, -1} }}
        }};

        // Here is `auto` used, because it's easier.
        template<Piece piece>
        static void generate_move_bitboards(){
            static_assert(piece == KING || piece == KNIGHT);

            auto& bitboards = piece == KING ? kingMoves : knightMoves;
            auto moves = piece == KING ? KING_MOVES : KNIGHT_MOVES;

            for (int rank = 0; rank < 8; rank++) {
                for (int file = 0; file < 8; file++) {
                    int square = get_square(rank, file);
                    set_bits_for_square<8, piece == KING ? 2 : 4>(moves, bitboards[square], rank, file);
                }
            }
        }

        // NOTE: This function uses kingMoves.
        template<Color color>
        static  void generate_pawn_bitboards(){
            auto& attack_moves = PAWN_ATTACK_MOVES[color];

            auto& quiet_moves_bb  = pawnQuietMoves[color];
            auto& attack_moves_bb = pawnAttackMoves[color];

            for (int rank = 1; rank < 7; rank++) {
                for (int file = 0; file < 8; file++) {
                    int square = get_square(rank, file);

                    if ((rank == 1 && color == Color::black()) || (rank == 6 && color == Color::white()))
                        set_bits_for_square<2, 2>(PAWN_QUIET_MOVES_2[color], quiet_moves_bb[square], rank, file);
                    else
                        set_bits_for_square<1, 1>(PAWN_QUIET_MOVES_1[color], quiet_moves_bb[square], rank, file);

                    set_bits_for_square<2, 2>(attack_moves, attack_moves_bb[square], rank, file);
                    // Valid area only [if king moves are correct, this will be correct :3]
                    attack_moves_bb[square] &= kingMoves[square];
                }
            }
        }

        template<int size, int max_dist>
        static void set_bits_for_square(const std::array<std::pair<int, int>, size>& moves,
                                 uint64_t& bitboard,
                                 int rank,
                                 int file){
            bitboard = 0ULL;
            for (const auto & move : moves){
                int tmp_rank = rank + move.first;
                int tmp_file = file + move.second;

                int bit_square = get_square(tmp_rank, tmp_file);

                int dist = std::abs(bit_square / 8 - rank) + std::abs((bit_square % 8) - file);
                if (dist > max_dist) continue;

                if (bit_square < 0 || bit_square > 63) continue;
                set_nth_bit(bitboard, bit_square);
            }
        }
    };
}

#endif //SIGMOID_MOVEGEN_HPP
