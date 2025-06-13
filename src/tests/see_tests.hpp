#ifndef SIGMOID_SEE_TESTS_HPP
#define SIGMOID_SEE_TESTS_HPP

#include "test.hpp"
#include "vector"
#include "../board.hpp"
#include "../movelist.hpp"
#include "test_helper.hpp"

using namespace Sigmoid;

struct SeeTests : public Test{
    std::string test_name() const override{
        return "SeeTests";
    }

    struct TestCase{
        TestCase(const std::string &fen,
                 int from,
                 int to,
                 int resultValue)
        : fen(fen), from(from), to(to), result(resultValue >= 0), resultValue(resultValue) {}

        std::string fen;
        int from;
        int to;
        bool result;
        int resultValue;
    };

    void run() const override{
        std::vector<TestCase> tests = {
                {"k3n3/ppp1pppp/8/2Pp4/8/3R4/P2Q4/K7 w - d6 0 2", 26, 19, 0},
                {"r2qkbn1/ppp1pp1p/3p1rp1/3Pn3/4P1b1/2N2P2/PPP2PPP/R1BQKB1R b KQq - 0 1", 38, 45,-200},
                {"7r/5qpk/2Qp1b1p/1N1r3n/BB3p2/5p2/P1P2P2/4RK1R w - - 0 1", 60,04,-500},
                {"6k1/1pp4p/p1pb4/6q1/3P1pRr/2P4P/PP1Br1P1/5RKN w - - 0 1", 51, 37, -100},
                {"5rk1/1pp2q1p/p1pb4/8/3P1NP1/2P5/1P1BQ1P1/5RK1 b - - 0 1",19, 37, 0},
                {"1r3r1k/p4pp1/2p1p2p/qpQP3P/2P5/3R4/PP3PP1/1K1R4 b - - 0 1",24, 48, -800},
                {"1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1", 43, 28,-200},
                {"r2qk1nr/ppp1ppbp/2b3p1/2p1p3/8/2N2N2/PPPP1PPP/R1BQR1K1 w kq - 0 1", 45,28,0}
        };

        for(const TestCase& test : tests){
            Board b;
            b.load_from_fen(test.fen);

            MoveList<false> ml(&b);
            Move m;
            while ((m = ml.get()) != Move::none()){
                if (m.from() == test.from && m.to() == test.to){
                    const bool result = b.see(m, 0);
                    throwable_assert(result, test.result);
                    break;
                }
            }
        }
    }
};



#endif //SIGMOID_SEE_TESTS_HPP
