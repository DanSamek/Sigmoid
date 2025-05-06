#ifndef SIGMOID_MOVEGEN_TESTS_HPP
#define SIGMOID_MOVEGEN_TESTS_HPP

#include "test.hpp"
#include "../movegen.hpp"

using namespace Sigmoid;

struct MovegenTests : public Test{
    std::string test_name() const override{
        return "MovegenTests";
    }

    void run() const override{
        bit_check();
    }

    void bit_check() const{
        Movegen mg;
        mg.init();
        //mg.print_bitboards<KNIGHT>();
        //mg.print_bitboards<KING>();
        mg.print_bitboards<PAWN>();
    }
};


#endif //SIGMOID_MOVEGEN_TESTS_HPP
