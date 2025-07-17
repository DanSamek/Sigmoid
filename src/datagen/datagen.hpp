#ifndef SIGMOID_DATAGEN_HPP
#define SIGMOID_DATAGEN_HPP

#include <thread>
#include <iomanip>
#include <fstream>
#include "../uci.hpp"

struct Datagen{
    struct Options {
        int softNodeLimit = 5000;
        int threadCount = 1;
    };

    static inline std::atomic<int64_t> totalPositions = 0;
    static inline std::atomic<int64_t> totalGames = 0;

    static inline bool stopSignal = false;
    static inline Timer timer = Timer(0,0,0,0,Color::BLACK);

    static void run() {
        Options options;
        read_options(options);
        Zobrist::init();
        Movegen::init();
        run_threads(options);
    }

    static void read_options(Options& options){
        // https://www.patorjk.com/software/taag/#p=display&f=Graffiti&t=Sigmoid
        std::cout << "  _________.__                      .__    .___\n";
        std::cout << " /   _____/|__| ____   _____   ____ |__| __| _/\n";
        std::cout << " \\_____  \\ |  |/ ___\\ /     \\ /  _ \\|  |/ __ | \n";
        std::cout << " /        \\|  / /_/  >  Y Y  (  <_> )  / /_/ | \n";
        std::cout << "/_______  /|__\\___  /|__|_|  /\\____/|__\\____ | \n";
        std::cout << "        \\/   /_____/       \\/               \\/ \n";
        std::cout << "Datagen tool" << std::endl;

        std::cout << "Soft node limit: ";
        std::cin >> options.softNodeLimit;
        std::cout << "Number of threads: ";
        std::cin >> options.threadCount;
    }

    static void set_seed(){
        auto seed = time(nullptr);
        std::cout << "run seed: " << seed << std::endl;
        srand(seed);
    }

    static void run_threads(const Options& options){
        std::vector<std::thread> workers;
        set_seed();
        for (int i = 0; i < options.threadCount; i++){
            workers.emplace_back([options, i](){ generate_positions(options, i);});
        }

        std::cout << "waiting on the `stop` message" << std::endl;
        std::string user_message;
        while(1){
            std::cin >> user_message;
            if(user_message != "stop")
                continue;

            stopSignal = true;
            break;
        }

        for (std::thread& worker : workers){
            worker.join();
        }
    }

    static std::string get_file_name(int workerId){
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        std::time_t current_time  = std::chrono::system_clock::to_time_t(now);
        std::tm* localTime = std::localtime(&current_time);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", localTime);

        oss << "sigmoid-data-worker-" << workerId << "-ts-"<< buffer << ".txt";
        std::string file_name = oss.str();
        return file_name;
    }

    static void generate_positions(const Options& options, int workerId){
        std::string file_name = get_file_name(workerId);
        std::ofstream file(file_name);
        file << std::fixed << std::setprecision(1);

        Board board;
        Engine engine;
        std::array<Move, MAX_POSSIBLE_MOVES> moves;
        Engine::DatagenOptions datagen_options;
        SearchResult search_result;
        datagen_options.softNodes = options.softNodeLimit;

        TranspositionTable* tt = new TranspositionTable();
        tt->resize(8);

        while (!stopSignal){
            loop_start:
            board.load_from_fen(Uci::START_POS);

            int random_move_count = (rand() % 50) >= 25 ? 8 : 9;
            while(random_move_count--){
                int num_generated_moves = 0;
                Movegen::generate_moves<false>(board.currentState, board.whoPlay, moves, num_generated_moves);
                int move_to_pick = rand() % num_generated_moves;
                int attempts = 1;
                if (!num_generated_moves)
                    goto loop_start;

                while(!board.make_move(moves[move_to_pick])){
                    move_to_pick = rand () % num_generated_moves;
                    if(attempts >= 3)
                        goto loop_start;
                    attempts++;
                }
            }

            // Initialize everything for a new game.
            tt->clear();
            engine.new_game(1);
            double game_result = 0; // Draw = 0.5, white = 1, black = 0

            datagen_options.tt = tt;
            datagen_options.board = &board;

            std::vector<std::pair<std::string, int>> positions;
            positions.reserve(100);

            while (1){
                if (board.is_draw()){
                    game_result = 0.5;
                    break;
                }

                search_result.bestMove = Move::none();
                search_result.score = MIN_VALUE;
                search_result.nodesVisited = 0;
                engine.datagen(datagen_options, &search_result);
                bool is_check = board.in_check();

                if (search_result.bestMove == Move::none()){
                    game_result = is_check ? (board.whoPlay ? 0 : 1) : 0.5;
                    break;
                }

                int abs_score = std::abs(search_result.score);
                int is_mate = abs_score >= CHECKMATE_BOUND;
                int white_relative_score = board.whoPlay ? search_result.score : -search_result.score;

                if (!is_check && !is_mate && !board.is_capture(search_result.bestMove)) {
                    positions.emplace_back(board.get_fen(), white_relative_score);
                }

                // Mate
                if(is_mate){
                    game_result = white_relative_score > 0;
                    break;
                }
                board.make_move(search_result.bestMove);
            }

            for (const auto &position: positions) {
                file << position.first << " | " << position.second << " | " << game_result << std::endl;
            }

            int64_t tmp = totalGames;
            totalGames++;
            totalPositions += positions.size();

            if (tmp % 100 == 0)
                print_status();
        }
        file.close();
    }

    static void print_status(){
        std::cout << "total games: " << totalGames << std::endl;
        std::cout << "positions generated: " << totalPositions << std::endl;
        std::cout << "positions per second: " << ((totalPositions * 1000) / (timer.get_ms()))  << std::endl;
    }
};


#endif //SIGMOID_DATAGEN_HPP
