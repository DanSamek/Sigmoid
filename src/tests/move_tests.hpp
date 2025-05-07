#ifndef SIGMOID_MOVE_TESTS_HPP
#define SIGMOID_MOVE_TESTS_HPP
#include <cassert>

#include "../move.hpp"
#include "test.hpp"

using namespace Sigmoid;

struct MoveTests : public Test{

    std::string test_name() const override{
        return "MoveTests";
    }

    void run() const override{

        Move m(10, 11);
        assert(m.from() == 10);
        assert(m.to() == 11);
        assert(m.special_type() == Move::NONE);

        m.set_data(62, 63);
        assert(m.from() == 62);
        assert(m.to() == 63);
        assert(m.special_type() == Move::NONE);

        m.set_data(0, 63, Move::PROMO_QUEEN);
        assert(m.from() == 0);
        assert(m.to() == 63);
        assert(m.special_type() == Move::PROMO_QUEEN);

        m.set_data(0, 63, Move::CASTLE);
        assert(m.from() == 0);
        assert(m.to() == 63);
        assert(m.special_type() == Move::CASTLE);

        m.set_data(23, 48, Move::EN_PASSANT);
        assert(m.from() == 23);
        assert(m.to() == 48);
        assert(m.special_type() == Move::EN_PASSANT);

        assert(Move::none().from() == 0);
        assert(Move::none().to() == 0);
        assert(Move::none().special_type() == Move::NONE);
    }

};


#endif //SIGMOID_MOVE_TESTS_HPP
