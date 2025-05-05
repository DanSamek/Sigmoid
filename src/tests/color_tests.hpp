#ifndef SIGMOID_COLOR_TESTS_HPP
#define SIGMOID_COLOR_TESTS_HPP

#include <cassert>

#include "test.hpp"
#include "../color.hpp"

using namespace Sigmoid;

struct ColorTests : public Test{

    std::string test_name() const override{
        return "ColorTests";
    }

    void run() const override{
        Color color;
        assert(color == Color::white());

        color = color.flip();
        assert(color == Color::black());

        color = color.flip();
        assert(color == Color::white());

        color = Color::black();
        assert(color == Color::black());
        assert(color.flip() == Color::white());
    }
};

#endif //SIGMOID_COLOR_TESTS_HPP
