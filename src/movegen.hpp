#ifndef SIGMOID_MOVEGEN_HPP
#define SIGMOID_MOVEGEN_HPP

#include <array>

#include "move.hpp"
#include "constants.hpp"
#include "board.hpp"
#include "magics.hpp"
#include "movelist.hpp"

namespace Sigmoid{
    // Pseudo-legal movegen.
    struct Movegen{
        // TODO ! captures.
        template<bool captures>
        static void generate_moves(const State& state, const bool whoPlay, MoveList& moveList) {
            if (whoPlay == Color::white()) generate_moves<captures, Color::white()>(state, moveList);
            else generate_moves<captures, Color::black()>(state, moveList);
        }

        template<bool captures, Color us>
        static void generate_moves(const State& state, MoveList& moveList){

            uint64_t friendly_bits = 0ULL;
            uint64_t enemy_bits = 0ULL;
            uint64_t merged_bits = 0ULL;

            for (const PairBitboard& pb : state.bitboards){
                friendly_bits |= pb.get<us>();
                enemy_bits    |= pb.get<~us>();
            }
            merged_bits = friendly_bits | enemy_bits;

            auto bitboard_to_moves = [&] (int fromSq, uint64_t bb, Move::SpecialType specialType = Move::NONE){
                int to_sq;
                while (bb && (to_sq = bit_scan_forward_pop_lsb(bb)) && !get_nth_bit(friendly_bits, to_sq)){
                    moveList.add(Move(fromSq, to_sq, specialType));
                }
            };

            auto filter_captures = [&](uint64_t& moves){
                if constexpr (captures){
                    moves &= enemy_bits;
                }
            };

            int pos;
            uint64_t moves;
            // Rook
            uint64_t bb = state.bitboards[ROOK].get<us>();
            while(bb && (pos = bit_scan_forward_pop_lsb(bb)) && (moves = Magics::get_rook_moves(merged_bits, pos))){
                filter_captures(moves);
                bitboard_to_moves(pos, moves);
            }

            // Bishop
            bb = state.bitboards[BISHOP].get<us>();
            while(bb && (pos = bit_scan_forward_pop_lsb(bb)) && (moves = Magics::get_bishop_moves(merged_bits, pos))){
                filter_captures(moves);
                bitboard_to_moves(pos, moves);
            }

            // Queen
            bb = state.bitboards[QUEEN].get<us>();
            while(bb && (pos = bit_scan_forward_pop_lsb(bb))){
                moves = Magics::get_bishop_moves(merged_bits, pos) | Magics::get_rook_moves(merged_bits, pos);
                filter_captures(moves);
                bitboard_to_moves(pos, moves);
            }

            // Knight
            bb = state.bitboards[KNIGHT].get<us>();
            while(bb && (pos = bit_scan_forward_pop_lsb(bb)) && (moves = knightMoves[pos])){
                filter_captures(moves);
                bitboard_to_moves(pos, moves);
            }

            // King
            pos = state.bitboards[KING].get<us>();
            bitboard_to_moves(kingMoves[pos]);
            const auto castlingMasks = CASTLING_FREE_MASKS[us];

            if (!captures && state.is_castling_set<us, false>() && (castlingMasks[K_CASTLE] & merged_bits) == 0){
                moveList.add(Move(pos, pos + 2, Move::CASTLE));
            }
            if (!captures && state.is_castling_set<us, true>() && (castlingMasks[Q_CASTLE] & merged_bits) == 0){
                moveList.add(Move(pos, pos - 2, Move::CASTLE));
            }

            // Pawns
            // Split to pawns:
            //  - simple move
            //  - pre-promotion rank
            //  - double move
            constexpr uint64_t pawn_double_push_bb = us == Color::white() ? WHITE_PAWN_START_BB : BLACK_PAWN_START_BB;
            constexpr uint64_t promo_ray_bb = us == Color::white() ? BLACK_PAWN_START_BB : WHITE_PAWN_START_BB;

            uint64_t double_push_pawns = state.bitboards[PAWN].get<us>() & pawn_double_push_bb;
            uint64_t promo_pawns       = state.bitboards[PAWN].get<us>() & promo_ray_bb;
            uint64_t simple_push_pawns = state.bitboards[PAWN].get<us>() ^ (double_push_pawns | promo_pawns);

            const uint64_t ep_bitmask = state.enPassantSquare != NO_SQUARE ? (1 << state.enPassantSquare) : 0ULL;

            while (simple_push_pawns && (pos = bit_scan_forward_pop_lsb(simple_push_pawns))) {
                bb = ((pawnQuietMoves[us][pos] & (~merged_bits)) * !captures)  | (pawnAttackMoves[us][pos] & enemy_bits);
                bitboard_to_moves(pos, bb);
                // en-passant.
                bb = pawnAttackMoves[us][pos] & ep_bitmask;
                bitboard_to_moves(pos, bb, Move::EN_PASSANT);
            }

            while (promo_pawns && (pos = bit_scan_forward_pop_lsb(promo_pawns))) {
                bb = ((pawnQuietMoves[us][pos] & (~merged_bits)) * !captures) | (pawnAttackMoves[us][pos] & enemy_bits);
                int to_sq;
                while (bb && (to_sq = bit_scan_forward_pop_lsb(bb))){
                    moveList.add(Move(pos, to_sq, Move::PROMO_BISHOP));
                    moveList.add(Move(pos, to_sq, Move::PROMO_KNIGHT));
                    moveList.add(Move(pos, to_sq, Move::PROMO_QUEEN));
                    moveList.add(Move(pos, to_sq, Move::PROMO_ROOK));
                }
            }

            while (double_push_pawns && (pos = bit_scan_forward_pop_lsb(double_push_pawns))){
                bb = (pawnAttackMoves[us][pos] & enemy_bits);

                uint64_t q_moves = ((pawnQuietMoves[us][pos] & (~merged_bits)) * !captures);
                const uint64_t opp_pawn_mask = pawnQuietMoves[~us][(us == Color::white() ? pos - 16 : pos + 16)];

                if ((q_moves & opp_pawn_mask) == 0ULL){
                    q_moves = 0ULL;
                }
                bb |= q_moves;
                bitboard_to_moves(pos, bb);
            }
        }

        void init(){
            Magics::init();
            generate_move_bitboards<KNIGHT>();
            generate_move_bitboards<KING>();

            generate_pawn_bitboards<Color::white()>();
            generate_pawn_bitboards<Color::black()>();
        }

        template<Color us>
        static bool is_square_attacked(const State& state, int square){
            uint64_t all = 0ULL;
            constexpr Color op = opp<us>();
            for (const PairBitboard& pb : state.bitboards){
                all |= pb.get<us>() | pb.get<op>();
            }

            if (Magics::get_rook_moves(all, square) & (state.bitboards[ROOK].get<op>() | state.bitboards[QUEEN].get<op>()))
                return true;

            if (Magics::get_bishop_moves(all, square) & (state.bitboards[BISHOP].get<op>() | state.bitboards[QUEEN].get<op>()))
                return true;

            if(knightMoves[square] & state.bitboards[KNIGHT].get<op>())
                return true;

            if(pawnAttackMoves[op][square] & state.bitboards[PAWN].get<op>())
                return true;

            if(kingMoves[square] & state.bitboards[KING].get<op>())
                return true;

            return false;
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
        static inline constexpr int Q_CASTLE = 0;
        static inline constexpr int K_CASTLE = 1;

        static inline constexpr uint64_t BLACK_PAWN_START_BB = 0xff00;
        static inline constexpr uint64_t WHITE_PAWN_START_BB = 0xff000000000000;

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
