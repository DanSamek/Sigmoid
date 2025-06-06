#include "tests/test_runner.hpp"

#include "uci.hpp"
#include "bencher.hpp"

int main(int argc, char* args[]) {
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

    // TODO datagen
    return 0;
}
