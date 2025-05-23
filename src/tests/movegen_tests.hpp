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
        Movegen::init();
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609);
        run_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 6, 119060324);
    }

    void run_perft(const std::string fen, int depth, int expected) const{
        Board board;
        board.load_from_fen(fen);
        long long result = move_recursion(board, depth);
        assert(result == expected);
    }

    long long move_recursion(Board& board, int depth) const{
        if (depth == 0) return 1;

        MoveList moveList;
        Movegen::generate_moves<false>(board.currentState, board.whoPlay, moveList);

        long long result = 0;
        Move move;
        while ((move = moveList.get()) != Move::none()){
            if (!board.make_move(move))
                continue;

            //board.print_state();
            result += move_recursion(board, depth - 1);

            board.undo_move();
        }
        return result;
    }

};


#endif //SIGMOID_MOVEGEN_TESTS_HPP
