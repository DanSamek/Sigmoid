#include <string>
#include <iostream>
#include <sstream>

#include "constants.hpp"
#include "movegen.hpp"
#include "board.hpp"
#include "engine.hpp"
#include "movegen.hpp"
#include "movelist.hpp"

#ifndef SIGMOID_UCI_HPP
#define SIGMOID_UCI_HPP

namespace Sigmoid{
    struct Uci{
        static inline const std::string START_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        int ttSize = 16;
        Board board;
        Engine engine;
        TranspositionTable tt;
        int threadCnt = 1;

        Uci() {
            tt = TranspositionTable();
            tt.resize(ttSize);
            board.load_from_fen(START_POS);
            engine.new_game(threadCnt);
        }

        void loop(){
            std::string line;
            while((std::getline(std::cin, line))){
                if (line == "quit")
                    break;
                if (line == "uci")
                    command_uci();
                if (line == "isready")
                    command_is_ready();
                if (line == "ucinewgame")
                    command_uci_new_game();
                if (line.find("position") != std::string::npos)
                    command_position(line);
                if (line.find("go") != std::string::npos)
                    command_go(line);
                if (line.find("setoption") != std::string::npos)
                    command_set_option(line);
                if (line.find("eval") != std::string::npos){
                    board.print_state();
                    std::cout << board.eval() << std::endl;
                }
            }
        }

        void command_set_option(const std::string& command) {
            std::istringstream stream (command);
            std::string type, value;
            stream >> type >> type >> type >> value >> value;

            if(type == "Hash"){
                ttSize = std::stoi(value);
                tt.resize(ttSize);
            }
            if (type == "Threads") {
                threadCnt = std::stoi(value);
                engine.new_game(threadCnt);
            }
        }

        // go wtime <> btime <> winc <> binc <>
        // go depth <>
        void command_go(const std::string& command){
            std::string tmp;
            std::istringstream iss(command);

            Engine::Options options;
            if (command.find("depth") != std::string::npos){
                iss >> tmp >> tmp >> options.depth;
            }
            else{
                iss >> tmp >> tmp >> options.wTime
                >> tmp >> options.bTime >> tmp >> options.wInc >> tmp >> options.bInc;
            }

            options.board = board;
            options.tt = &tt;
            engine.start_searching(options);
        }

        // position startpos moves <>
        // position fen <fen> moves <>
        void command_position(const std::string& command){
            std::istringstream iss(command);

            if (command.find("startpos") != std::string::npos){
                board.load_from_fen(START_POS);
            }
            else{
                unsigned long from = command.find("fen");
                unsigned long to = command.find("moves");

                std::string fen = to == std::string::npos ?
                        command.substr(from + 4) : command.substr(from + 4, (to - from - 5));
                board.load_from_fen(fen);
            }
            unsigned long pos = command.find("moves");
            if (pos == std::string::npos)
                return;

            std::string moves = command.substr(pos + 5);
            std::istringstream moves_stream(moves);
            std::string str_move;
            while ((moves_stream >> str_move)){
                if (str_move.empty())
                    continue;

                MoveList<false> mp(&board);
                Move move;

                while ((move = mp.get()) != Move::none()){
                    if (move.to_uci().find(str_move) == std::string::npos)
                        continue;

                    if (!board.make_move(move))
                        std::cout << "Illegal move.";
                    break;
                }
            }
        }

        void command_uci(){
            std::cout << "id name Sigmoid " << VERSION << std::endl;
            std::cout << "id author Daniel Samek" << std::endl;

            std::cout << "option name Hash type spin default " << ttSize << " min 1 max 128000" << std::endl;
            std::cout << "option name Threads type spin default 1 min 1 max 1024" << std::endl;
            std::cout << "uciok" << std::endl;
        }

        void command_is_ready(){
            Zobrist::init();
            Movegen::init();
            std::cout << "readyok" << std::endl;
        }

        void command_uci_new_game(){
            board.load_from_fen(START_POS);
            tt.resize(ttSize);
            engine.new_game(threadCnt);
        }
    };
}

#endif //SIGMOID_UCI_HPP
