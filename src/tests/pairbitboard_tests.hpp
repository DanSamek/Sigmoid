#ifndef SIGMOID_PAIRBITBOARD_TESTS_HPP
#define SIGMOID_PAIRBITBOARD_TESTS_HPP

#include <cassert>

#include "test.hpp"
#include "../pairbitboard.hpp"
#include "../color.hpp"

using namespace Sigmoid;

struct PairBitboardTests : public Test{

    std::string test_name() const override{
        return "PairBitboardTests";
    }

    void run() const override{
        PairBitboard pb;

        pb.set_bit<WHITE>(5);
        pb.set_bit<BLACK>(4);

        assert(pb.get<WHITE>(5) == 1);
        assert(pb.get<BLACK>(5) == 0);

        assert(pb.get<WHITE>(4) == 0);
        assert(pb.get<BLACK>(4) == 1);

        pb.pop_bit<WHITE>(5);
        pb.pop_bit<BLACK>(4);

        assert(pb.get<WHITE>(5) == 0);
        assert(pb.get<BLACK>(4) == 0);
    }
};

#endif //SIGMOID_PAIRBITBOARD_TESTS_HPP
