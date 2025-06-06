#ifndef SIGMOID_TEST_HELPER_HPP
#define SIGMOID_TEST_HELPER_HPP

#include "ostream"

template<class T>
static inline void throwable_assert(T current, T expected) {
    if (current == expected) return;

    std::ostringstream oss;
    oss << "assert failed: " << current <<  " != " << expected;
    throw std::out_of_range(oss.str());
}

#endif //SIGMOID_TEST_HELPER_HPP
