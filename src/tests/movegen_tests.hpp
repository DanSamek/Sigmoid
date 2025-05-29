#ifndef SIGMOID_MOVEGEN_TESTS_HPP
#define SIGMOID_MOVEGEN_TESTS_HPP

#include <chrono>

#include "test.hpp"
#include "../movegen.hpp"

using namespace Sigmoid;

struct MovegenTests : public Test{
    std::string test_name() const override{
        return "MovegenTests";
    }


    void verify_board(const Board& board, int depth) const{
        auto verify_bb = [&](uint64_t bb, Piece pc) ->void{
            int bit;
            while (bb && (bit = bit_scan_forward_pop_lsb(bb))){
                if (board.currentState.pieceMap[bit] == pc)
                    continue;

                board.print_state();
                std::cout << depth << std::endl;
                assert(board.currentState.pieceMap[bit] == pc);
                throw std::out_of_range("nah");
            }
        };

        int piece_cnt = 0;
        for (int i = Piece::PAWN; i <= Piece::KING; ++i){
            const PairBitboard& pbb = board.currentState.bitboards[i];
            verify_bb(pbb.get<WHITE>(), Piece(i));
            verify_bb(pbb.get<BLACK>(), Piece(i));

            uint64_t tmp = pbb.get<WHITE>() | pbb.get<BLACK>();
            piece_cnt += count_bits(tmp);
        }

        int non_piece_cnt = 0;
        for (int square = 0; square < 64; ++square){
            non_piece_cnt += board.currentState.pieceMap[square] == Piece::NONE;
        }
        if (non_piece_cnt + piece_cnt != 64){
            throw std::out_of_range("nah");
        }
        assert(non_piece_cnt + piece_cnt == 64);
    }

    void run() const override{
        Movegen::init();
        run_perft("8/1p4p1/8/q1PK1P1r/3p1k2/8/4P3/4Q3 b - - 0 1", 5, 6323457 );
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",5,4865609 );
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",6,119060324 );
        run_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 6,11030083 );
        run_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 7,178633661 );
        run_perft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",5, 164075551 );
        run_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 5, 89941194 );
        run_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0", 5, 193690690 );
        run_perft("8/8/8/1PpK4/5p2/4k3/8/8 b - - 0 24", 9, 133225511 );
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 6, 119060324);

        run_perft("rnbqkbnr/ppppp1pp/8/8/5pP1/P1P5/1P1PPP1P/RNBQKBNR b KQkq g3 0 3", 1, 21 );
        run_perft("rnbqkbnr/ppppp1pp/8/8/5pP1/P1P5/1P1PPP1P/RNBQKBNR b KQkq g3 0 3", 3, 9365 );
        run_perft("rnbqkbnr/ppppp1pp/8/8/4Pp2/P1P5/1P1P1PPP/RNBQKBNR b KQkq e3 0 3",    1, 21 );
        run_perft("rnbqkbnr/ppppp1pp/8/8/4Pp2/P1P5/1P1P1PPP/RNBQKBNR b KQkq e3 0 3", 3, 13010 );
        run_perft("1r2k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/1PN2Q1p/2PBBPPP/R3K2R b KQk a3 0 8", 1, 44 );
        run_perft("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 1, 15 );

        // run_perft("2B1k2r/p4pp1/1N1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R b KQk - 0 5", 2, 709 );
        run_perft("2r1k2r/pP3pp1/1N1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R w KQk - 1 5", 3, 38982 );
        run_perft("r3k2r/pP3pp1/1N1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R b KQkq - 0 4", 4, 671685 );
        run_perft("r3kr2/pP3pp1/1N1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R w KQq - 1 5", 1,41 );
        run_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0", 3, 97862 );
        run_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0", 4, 4085603 );
        run_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/1PN2Q1p/P1PBBPPP/R3K2R b KQkq - 0 7", 3, 81066 );
        run_perft("r4rk1/p1ppqpb1/bn2pnp1/3PN3/1p2P3/1PN2Q1p/P1PBBPPP/R3K2R w KQ - 1 8", 2, 1814 );
        run_perft("r3k3/p1ppqpbr/bn2pnp1/3PN3/1p2P3/1PN2Q1p/P1PBBPPP/R3K2R w KQq - 1 8", 2, 1812 );
        run_perft("1r2k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/1PN2Q1p/P1PBBPPP/R3K2R w KQk - 1 8", 2, 2000 );
        run_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/5Q2/PPPBBPpP/R2NK2R w KQkq - 0 7", 1, 47 );
        run_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/5Q1p/PPPBBPPP/R2NK2R b KQkq - 1 6", 2, 2040 );
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902);
        run_perft("r1bqkbnr/pppppppp/n7/8/8/7P/PPPPPPPR/RNBQKBN1 b Qkq - 2 2", 1, 20);
        run_perft("r1bqkbnr/pppppppp/n7/8/8/7P/PPPPPPP1/RNBQKBNR w KQkq - 1 2", 2, 380);
        run_perft("rnbqkbnr/pppppppp/8/8/8/7P/PPPPPPP1/RNBQKBNR b KQkq - 0 1", 3, 8457);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281);
        run_perft("rnbqkbnr/p1pppppp/8/Pp6/8/8/1PPPPPPP/RNBQKBNR b KQkq - 0 2", 2, 436);
        run_perft("rnbqkbnr/p1pppppp/8/1p6/P7/8/1PPPPPPP/RNBQKBNR w KQkq b6 0 2", 3, 11606);
        run_perft("rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq a3 0 1", 4, 217832);
        run_perft("rnbqkbnr/p1pppppp/8/8/1p1P4/8/PPPKPPPP/RNBQ1BNR w kq - 0 3", 1, 22);
        run_perft("rnbqkbnr/p1pppppp/8/1p6/3P4/8/PPPKPPPP/RNBQ1BNR b kq - 1 2", 2, 503);
        run_perft("rnbqkbnr/p1pppppp/8/1p6/3P4/8/PPP1PPPP/RNBQKBNR w KQkq b6 0 2", 3, 17127);
        run_perft("rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 1", 4, 361790);
        run_perft("r1bqkbnr/p1pppppp/np6/8/8/4PQ2/PPPP1PPP/RNB1KBNR w KQkq - 0 3", 1, 39);
        run_perft("r1bqkbnr/pppppppp/n7/8/8/4PQ2/PPPP1PPP/RNB1KBNR b KQkq - 2 2", 2, 753);
        run_perft("r1bqkbnr/pppppppp/n7/8/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 1 2", 3, 18272);
        run_perft("rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1", 4, 402988);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609);
        run_perft("r3k3/pP3pp1/1N1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2r w Qq - 0 5", 1,2 );
        run_perft("r3k2r/p1ppqpb1/bn2p1p1/3PN3/1p2P1n1/5Q1p/PPPBBPPP/R2NK2R w KQkq - 2 7", 1, 48 );
        run_perft("r3k2r/p1ppqpb1/bn1Ppnp1/4N3/1p2P3/2N2Q2/PPPBBPpP/R3K2R w KQkq - 0 6", 1, 47 );
        run_perft("8/8/8/1PpK4/5p2/4k3/8/8 b - - 0 24", 1, 7);
        run_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0", 1, 48 );
        run_perft("r3k2r/p1ppqpb1/bn1Ppnp1/4N3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 5", 2, 1991 );
        run_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0", 2, 2039 );
        run_perft("r3k2r/pP3pp1/1N1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R b KQkq - 0 4", 2,913 );
        run_perft("r3k2r/pp3pp1/PN1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R w KQkq - 4 4", 1,34 );
        run_perft("r3k2r/pp3pp1/PN1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R w KQkq - 4 4", 2,751 );
        run_perft("r3k2r/pp3pp1/PN1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R w KQkq - 4 4", 3,23544 );
        run_perft("r3k2r/pp3pp1/PN1pr1p1/4p1P1/4P3/3P4/P1P2PP1/R3K2R w KQkq - 4 4", 5, 15587335 );
        run_perft("1r2k2r/1p3pp1/Pp1pr1p1/4p1P1/P3P3/3P4/2P2PP1/1R2K2R b Kk - 0 6",1,24 );
        run_perft("1r2k2r/1p3pp1/Pp1pr1p1/4p1P1/4P3/P2P4/2P2PP1/1R2K2R w Kk - 1 6",2,726 );
        run_perft("r3k2r/1p3pp1/Pp1pr1p1/4p1P1/4P3/P2P4/2P2PP1/1R2K2R b Kkq - 0 5",3,20168 );
        run_perft("8/2p5/3p4/1P5r/KR3p2/2k5/4P1P1/8 w - - 2 2",5,982226 );
        run_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 4,43238 );
        run_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 1,14 );
        run_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 2,191 );
        run_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 3,2812 );
        run_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 5,674624 );
        run_perft("rnQ1qk1r/pp3ppp/2p5/8/1bB5/8/PPP1NKPP/RNBQ3R w - - 1 10", 1,56 );
        run_perft("rnQq1k1r/pp3ppp/2p5/8/1bB5/8/PPP1NKPP/RNBQ3R b - - 0 9", 2,1396 );
        run_perft("rnQq1k1r/pp3ppp/2p5/8/1bB5/2P5/PP2NnPP/RNBQK2R b KQ - 0 9",2,1454 );
        run_perft("rnQq1k1r/pp3ppp/2p5/8/1bB5/8/PPP1NnPP/RNBQK2R w KQ - 1 9",1,8 );
        run_perft("rnQq1k1r/pp3ppp/2p5/8/1bB5/8/PPP1NnPP/RNBQK2R w KQ - 1 9",2,249 );
        run_perft("rnQq1k1r/pp3ppp/2p5/8/1bB5/8/PPP1NnPP/RNBQK2R w KQ - 1 9",3,10709 );
        run_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",2,1486 );
        run_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",3,62379 );
        run_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",4,2103487 );
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",2,400 );
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",3,8902 );
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",4,197281 );
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609 );
        run_perft("2b1b3/1r1P4/3K3p/1p6/2p5/6k1/1P3p2/4B3 w - - 0 42", 5, 5617302 );
        run_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 6, 11030083 );
        run_perft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594 );
        run_perft("r3k1nr/p2pp1pp/b1n1P1P1/1BK1Pp1q/8/8/2PP1PPP/6N1 w kq - 0 1", 4, 497787 );
        run_perft("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 6, 1134888 );
        run_perft("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 6, 1440467 );
        run_perft("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 6, 661072 );
        run_perft("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 7, 15594314 );
        run_perft("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 4, 1274206 );
        run_perft("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 5, 58773923 );
        run_perft("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 6, 3821001 );
        run_perft("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 5, 1004658 );
        run_perft("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 6, 217342 );
        run_perft("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 6, 92683 );
        run_perft("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 10, 5966690 );
        run_perft("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 7, 567584 );
        run_perft("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 6, 3114998 );
        run_perft("r1bq2r1/1pppkppp/1b3n2/pP1PP3/2n5/2P5/P3QPPP/RNB1K2R w KQ a6 0 12", 5, 42761834 );
        run_perft("r3k2r/pppqbppp/3p1n1B/1N2p3/1nB1P3/3P3b/PPPQNPPP/R3K2R w KQkq - 11 10", 4, 3050662 );
        run_perft("4k2r/1pp1n2p/6N1/1K1P2r1/4P3/P5P1/1Pp4P/R7 w k - 0 6", 5, 10574719 );
        run_perft("1Bb3BN/R2Pk2r/1Q5B/4q2R/2bN4/4Q1BK/1p6/1bq1R1rb w - - 0 1", 4, 6871272 );
        run_perft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 6, 71179139 );
        run_perft("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", 6, 28859283 );
        run_perft("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", 9, 7618365 );
        run_perft("3r4/2p1p3/8/1P1P1P2/3K4/5k2/8/8 b - - 0 1", 4, 28181 );


    }

    void run_perft(const std::string fen, int depth, int expected) const{
        Board board;
        std::cout << fen << std::endl;
        board.load_from_fen(fen);
        //board.print_state();
        auto start = std::chrono::high_resolution_clock::now();
        long long result = move_recursion<false>(board, depth, depth);
        if (result == expected){
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if (!duration) duration = 1;

            std::cout << result << " == " << expected << std::endl;
            std::cout << "nodes per second: " << (result / duration) * 1000 << " time: " << duration <<  std::endl;
        }
        else{
            throw std::out_of_range("nah");
        }
    }

    template<bool debug>
    long long move_recursion(Board& board, int depth, const int maxDepth) const{
        if (depth == 0) return 1;

        [[maybe_unused]] std::string space;
        if constexpr (debug){
            std::ostringstream space_stream;
            int spaces = maxDepth - depth;
            while (spaces--)
                space_stream << "       ";
            space = space_stream.str();
        }

        MoveList move_list;
        Movegen::generate_moves<false>(board.currentState, board.whoPlay, move_list);

        long long result = 0;
        Move move;

        while ((move = move_list.get()) != Move::none()){
            if (!board.make_move(move))
                continue;

            if constexpr (debug){
                verify_board(board, depth);
            }

            auto rec_result = move_recursion<debug>(board, depth - 1, maxDepth);
            result += rec_result;
            board.undo_move();

            if constexpr (debug){
                if (depth == maxDepth) {
                    std::ostringstream oss;
                    oss << space << move.to_uci() << ":" << rec_result << std::endl;
                    std::cout << oss.str();
                }
            }
        }
        return result;
    }

};


#endif //SIGMOID_MOVEGEN_TESTS_HPP
