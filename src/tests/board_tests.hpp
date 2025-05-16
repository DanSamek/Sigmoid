#ifndef SIGMOID_BOARD_TESTS_HPP
#define SIGMOID_BOARD_TESTS_HPP

#include "test.hpp"
#include "../board.hpp"

using namespace Sigmoid;

struct BoardTests : public Test{
    std::string test_name() const override{
        return "BoardTests";
    }


    void run() const override{
        // Run and see [not ideal, but who cares at all].
        Board b;
        b.load_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        b.print_state();

        b.load_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
        b.print_state();

        b.load_from_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
        b.print_state();

        b.load_from_fen("1r2r1k1/2qb1p2/p2p1b2/1pnPp2Q/2p1P2p/P1P1B2P/1PB2PP1/4RRK1 w - - 3 26");
        b.print_state();

        b.load_from_fen("rnbq1rk1/ppp1bp2/5np1/3pp2p/4P2N/5PPP/PPPP3R/RNBQKB2 w Q - 1 8");
        b.print_state();
    }
};

#endif //SIGMOID_BOARD_TESTS_HPP
