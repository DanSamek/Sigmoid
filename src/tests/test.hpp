#ifndef SIGMOID_TEST_HPP
#define SIGMOID_TEST_HPP

#include "string"

struct Test{
    virtual std::string test_name() const = 0;
    virtual void run() const = 0;
};

#endif //SIGMOID_TEST_HPP
