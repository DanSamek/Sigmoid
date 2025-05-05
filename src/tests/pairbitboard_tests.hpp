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

        pb.set_bit<Color::white()>(5);
        pb.set_bit<Color::black()>(4);

        assert(pb.get<Color::white()>(5) == 1);
        assert(pb.get<Color::black()>(5) == 0);

        assert(pb.get<Color::white()>(4) == 0);
        assert(pb.get<Color::black()>(4) == 1);

        pb.pop_bit<Color::white()>(5);
        pb.pop_bit<Color::black()>(4);

        assert(pb.get<Color::white()>(5) == 0);
        assert(pb.get<Color::black()>(4) == 0);
    }
};

#endif //SIGMOID_PAIRBITBOARD_TESTS_HPP
