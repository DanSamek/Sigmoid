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
        Color color = Color::WHITE;
        assert(color == WHITE);

        color = ~color;
        assert(color == BLACK == ~(~color));

        color = ~color;
        assert(color == WHITE);

        color = BLACK;
        assert(color == BLACK);
        assert(~color == WHITE);

        color = WHITE;
        assert(~color == ~WHITE);

    }
};

#endif //SIGMOID_COLOR_TESTS_HPP
