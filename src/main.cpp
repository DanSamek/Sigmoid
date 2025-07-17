#include "tests/test_runner.hpp"
#include "datagen/datagen.hpp"

#include "uci.hpp"
#include "bencher.hpp"
#include "nnue/old_arch/nnue.hpp"

int main(int argc, char* args[]) {

    OldNNUE nnue;


    if (argc == 1){
        Uci uci;
        uci.loop();
        return EXIT_SUCCESS;
    }
    std::string command(args[1]);
    if (command == "test")
        TestRunner::run_all();
    if (command == "bench")
        Bencher::bench();
    if (command == "datagen")
        Datagen::run();
    return 0;
}
