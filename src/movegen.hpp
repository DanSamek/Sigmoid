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
        static void generate_moves(const State& state, const Color whoPlay, std::array<Move, MAX_POSSIBLE_MOVES>& moves, int& size) {
            if (whoPlay == WHITE) generate_moves_<captures, WHITE>(state, moves, size);
            else generate_moves_<captures, BLACK>(state, moves, size);
        }



        static inline bool ready = false;
        static void init(){
            if (ready) return;
            Magics::init();
            generate_move_bitboards<KNIGHT>();
            generate_move_bitboards<KING>();

            generate_pawn_bitboards<WHITE>();
            generate_pawn_bitboards<BLACK>();
            ready = true;
        }

        template<Color us>
        inline static bool is_square_attacked(const State& state, int square, uint64_t all = 0xffffffffffffffff){
            constexpr Color op = ~us;
            if (all == 0xffffffffffffffff)
            {
                all = 0ULL;
                for (const PairBitboard& pb : state.bitboards){
                    all |= pb.get<us>() | pb.get<op>();
                }
            }

            if (Magics::get_rook_moves(all, square) & (state.bitboards[ROOK].get<op>() | state.bitboards[QUEEN].get<op>()))
                return true;

            if (Magics::get_bishop_moves(all, square) & (state.bitboards[BISHOP].get<op>() | state.bitboards[QUEEN].get<op>()))
                return true;

            if(knightMoves[square] & state.bitboards[KNIGHT].get<op>())
                return true;

            if(pawnAttackMoves[us][square] & state.bitboards[PAWN].get<op>())
                return true;

            if(kingMoves[square] & state.bitboards[KING].get<op>())
                return true;

            return false;
        }

    private:

        template<bool captures>
        inline static void filter_captures(uint64_t& moves, const uint64_t& enemy_bits){
            if constexpr (captures){
                moves &= enemy_bits;
            }
        }

        template<bool captures, Color us>
        static void generate_moves_(const State& state, std::array<Move, MAX_POSSIBLE_MOVES>& movesRef, int& size){

            uint64_t friendly_bits = 0ULL;
            uint64_t enemy_bits = 0ULL;
            uint64_t merged_bits = 0ULL;

            for (const PairBitboard& pb : state.bitboards){
                friendly_bits |= pb.get<us>();
                enemy_bits    |= pb.get<~us>();
            }
            merged_bits = friendly_bits | enemy_bits;

            auto add = [&](const Move& move){
                movesRef[size++] = move;
            };

            auto bitboard_to_moves = [&] (int fromSq, uint64_t bb, Move::SpecialType specialType = Move::NONE){
                int to_sq;
                while (bb){
                    to_sq = bit_scan_forward_pop_lsb(bb);
                    if (!get_nth_bit(friendly_bits, to_sq))
                        add(Move(fromSq, to_sq, specialType));
                }
            };

            int pos;
            uint64_t moves;
            // Rook
            uint64_t bb = state.bitboards[ROOK].get<us>();
            while(bb){
                pos = bit_scan_forward_pop_lsb(bb);
                moves = Magics::get_rook_moves(merged_bits, pos);
                filter_captures<captures>(moves, enemy_bits);
                bitboard_to_moves(pos, moves);
            }

            // Bishop
            bb = state.bitboards[BISHOP].get<us>();
            while(bb){
                pos = bit_scan_forward_pop_lsb(bb);
                moves = Magics::get_bishop_moves(merged_bits, pos);
                filter_captures<captures>(moves, enemy_bits);
                bitboard_to_moves(pos, moves);
            }

            // Queen
            bb = state.bitboards[QUEEN].get<us>();
            while(bb){
                pos = bit_scan_forward_pop_lsb(bb);
                moves = Magics::get_bishop_moves(merged_bits, pos) | Magics::get_rook_moves(merged_bits, pos);
                filter_captures<captures>(moves, enemy_bits);
                bitboard_to_moves(pos, moves);
            }

            // Knight
            bb = state.bitboards[KNIGHT].get<us>();
            while(bb){
                pos = bit_scan_forward_pop_lsb(bb);
                moves = knightMoves[pos];
                filter_captures<captures>(moves, enemy_bits);
                bitboard_to_moves(pos, moves);
            }

            // King
            bb = state.bitboards[KING].get<us>();
            pos = bit_scan_forward_pop_lsb(bb);
            bitboard_to_moves(pos, kingMoves[pos]);
            const auto castlingMasks = CASTLING_FREE_MASKS[us];

            if (!captures && state.is_castling_set<us, false>() && (castlingMasks[K_CASTLE] & merged_bits) == 0){
                add(Move(pos, pos + 2, Move::CASTLE));
            }
            if (!captures && state.is_castling_set<us, true>() && (castlingMasks[Q_CASTLE] & merged_bits) == 0){
                add(Move(pos, pos - 2, Move::CASTLE));
            }

            // Pawns
            // Split to pawns:
            //  - simple move
            //  - pre-promotion rank
            //  - double move
            constexpr int64_t pawn_double_push_bb =  PAWN_STARTS[us];
            constexpr uint64_t promo_ray_bb = PAWN_STARTS[~us];

            uint64_t double_push_pawns = state.bitboards[PAWN].get<us>() & pawn_double_push_bb;
            uint64_t promo_pawns       = state.bitboards[PAWN].get<us>() & promo_ray_bb;
            uint64_t simple_push_pawns = state.bitboards[PAWN].get<us>() ^ (double_push_pawns | promo_pawns);

            const uint64_t ep_bitmask = state.enPassantSquare != NO_SQUARE ? (1ULL << state.enPassantSquare) : 0ULL;

            while (simple_push_pawns) {
                pos = bit_scan_forward_pop_lsb(simple_push_pawns);
                bb = ((pawnQuietMoves[us][pos] & (~merged_bits)) * !captures)  | (pawnAttackMoves[us][pos] & enemy_bits);
                bitboard_to_moves(pos, bb);
                // en-passant.
                bb = pawnAttackMoves[us][pos] & ep_bitmask;
                bitboard_to_moves(pos, bb, Move::EN_PASSANT);
            }

            while (promo_pawns) {
                pos = bit_scan_forward_pop_lsb(promo_pawns);
                bb = ((pawnQuietMoves[us][pos] & (~merged_bits)) * !captures) | (pawnAttackMoves[us][pos] & enemy_bits);
                int to_sq;
                while (bb){
                    to_sq = bit_scan_forward_pop_lsb(bb);
                    add(Move(pos, to_sq, Move::PROMO_BISHOP));
                    add(Move(pos, to_sq, Move::PROMO_KNIGHT));
                    add(Move(pos, to_sq, Move::PROMO_QUEEN));
                    add(Move(pos, to_sq, Move::PROMO_ROOK));
                }
            }

            while (double_push_pawns){
                pos = bit_scan_forward_pop_lsb(double_push_pawns);
                bb = (pawnAttackMoves[us][pos] & enemy_bits);

                uint64_t q_moves = ((pawnQuietMoves[us][pos] & (~merged_bits)) * !captures);
                uint64_t opp_pawn_mask = pawnQuietMoves[~us][(us == WHITE ? pos - 16 : pos + 16)];

                if ((q_moves & opp_pawn_mask) == 0ULL){
                    q_moves = 0ULL;
                }
                bb |= q_moves;
                bitboard_to_moves(pos, bb);
            }
        }


    private:
        static inline constexpr int Q_CASTLE = 0;
        static inline constexpr int K_CASTLE = 1;

        static inline constexpr uint64_t BLACK_PAWN_START_BB = 0xff00;
        static inline constexpr uint64_t WHITE_PAWN_START_BB = 0xff000000000000;

        static inline constexpr uint64_t PAWN_STARTS[2] = {WHITE_PAWN_START_BB, BLACK_PAWN_START_BB};

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

            for (int rank = 0; rank < 8; rank++) {
                for (int file = 0; file < 8; file++) {
                    int square = get_square(rank, file);

                    if ((rank == 1 && color == BLACK) || (rank == 6 && color == WHITE))
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
