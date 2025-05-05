#ifndef SIGMOID_HELPER_HPP
#define SIGMOID_HELPER_HPP

namespace Sigmoid {
    inline int get_square(int rank, int file){
        return rank * 8 + file;
    }
}

#endif //SIGMOID_HELPER_HPP
