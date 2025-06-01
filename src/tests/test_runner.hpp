#ifndef SIGMOID_TEST_RUNNER_HPP
#define SIGMOID_TEST_RUNNER_HPP

#include <vector>
#include <iostream>
#include <memory>

#include "test.hpp"
#include "move_tests.hpp"
#include "color_tests.hpp"
#include "pairbitboard_tests.hpp"
#include "board_tests.hpp"
#include "movegen_tests.hpp"
#include "zobrist_tests.hpp"

// No lib used for tests.
// Most of the tests are just sanity checks.
struct TestRunner{
    static void run_all(){
        srand(17);

        std::vector<std::unique_ptr<Test>> tests;
        tests.push_back(std::make_unique<MoveTests>());
        tests.push_back(std::make_unique<ColorTests>());
        tests.push_back(std::make_unique<PairBitboardTests>());
        tests.push_back(std::make_unique<BoardTests>());
        tests.push_back(std::make_unique<ZobristTests>());
        tests.push_back(std::make_unique<MovegenTests>());

        Zobrist::init();
        Movegen::init();

        for (std::unique_ptr<Test>& test: tests){
            std::cout << "Running test " << test->test_name() << "." << std::endl;
            test->run();
            std::cout << "Test " << test->test_name() << " passed." << std::endl;
            std::cout << std::endl;
        }
    }
};

#endif //SIGMOID_TEST_RUNNER_HPP
