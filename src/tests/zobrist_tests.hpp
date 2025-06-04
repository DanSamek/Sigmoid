#ifndef SIGMOID_ZOBRIST_TESTS_HPP
#define SIGMOID_ZOBRIST_TESTS_HPP

#include <iostream>

#include "test.hpp"
#include "../zobrist.hpp"
#include "../board.hpp"
#include "../movelist.hpp"

using namespace Sigmoid;

struct ZobristTests : public Test{

    std::string test_name() const override{
        return "ZobristTests";
    }

    static inline void throwable_assert(bool condition) {
        if (condition) return;
        throw std::out_of_range("assert failed.");
    }

    void run() const override{
        Board b;
        b.load_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        uint64_t initHash = b.currentState.zobristKey;

        MoveList<false> moves(&b);

        uint64_t zobrist = b.currentState.zobristKey;
        throwable_assert(b.make_move(moves.get()) == true);
        throwable_assert(zobrist != b.currentState.zobristKey);

        b.undo_move();

        throwable_assert(zobrist == b.currentState.zobristKey);
        throwable_assert(initHash == zobrist);
        throwable_assert(initHash == b.currentState.zobristKey);


        // 2 MOVES.
        throwable_assert(b.make_move(moves.get()) == true);
        uint64_t zobrist1 = b.currentState.zobristKey;

        MoveList<false> moves2(&b);

        throwable_assert(b.make_move(moves2.get()) == true);

        uint64_t zobrist2 = b.currentState.zobristKey; // uniq
        b.undo_move();

        throwable_assert(b.currentState.zobristKey == zobrist1);
        b.undo_move();

        throwable_assert(b.currentState.zobristKey == zobrist);
        throwable_assert(zobrist2 != zobrist && zobrist2 != zobrist1);

        // try simple position with different moves.
        b.load_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        // load init pos
        throwable_assert(b.currentState.zobristKey == initHash);


        MoveList<false> moves3(&b);

        // try play moves.
        // from = 52, double pawn up
        Move move = try_find_move(moves3, 52, 36);
        b.make_move(move);
        b.print_state();

        // from = 1, to 18
        moves3 = MoveList<false>(&b);
        move = try_find_move(moves3, 1, 18);
        b.make_move(move);
        b.print_state();

        // from = 62 to 45
        moves3 = MoveList<false>(&b);
        move = try_find_move(moves3, 62, 45);
        b.make_move(move);
        b.print_state();

        // from = 11 to 27
        moves3 = MoveList<false>(&b);
        move = try_find_move(moves3, 11, 27);
        b.make_move(move);
        b.print_state();

        uint64_t patternHash = b.currentState.zobristKey;

        // reset.
        b.load_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        throwable_assert(b.currentState.zobristKey == initHash);

        // from = 62 to 45
        moves3 = MoveList<false>(&b);
        move = try_find_move(moves3, 62, 45);
        b.make_move(move);
        b.print_state();

        // from = 1, to 18
        moves3 = MoveList<false>(&b);
        move = try_find_move(moves3, 1, 18);
        b.make_move(move);
        b.print_state();

        // from = 52, double pawn up
        moves3 = MoveList<false>(&b);
        move = try_find_move(moves3, 52, 36);
        b.make_move(move);
        b.print_state();

        // from = 11 to 27
        moves3 = MoveList<false>(&b);
        move = try_find_move(moves3, 11, 27);
        b.make_move(move);
        b.print_state();


        throwable_assert(b.currentState.zobristKey == patternHash);


        // end-game shuffling with pieces.
        b.load_from_fen("7r/8/8/4k3/8/4K3/8/R7 w - - 0 1");
        b.print_state();
        initHash = b.currentState.zobristKey;

        moves3 = MoveList<false>(&b);
        b.make_move(try_find_move(moves3, 56, 0)); // UP w rook

        moves3 = MoveList<false>(&b);
        b.make_move(try_find_move(moves3, 7, 63)); // DOWN b rook
        throwable_assert(initHash != b.currentState.zobristKey);
        b.print_state();

        moves3 = MoveList<false>(&b);
        b.make_move(try_find_move(moves3, 0, 56)); // DOWN w rook

        moves3 = MoveList<false>(&b);
        b.make_move(try_find_move(moves3, 63,7)); // UP b rook

        b.print_state();

        throwable_assert(initHash == b.currentState.zobristKey);
        throwable_assert(!b.is_draw());
        // try 3-fold repetition.

        moves3 = MoveList<false>(&b);
        b.make_move(try_find_move(moves3, 56, 0)); // UP w rook

        moves3 = MoveList<false>(&b);
        b.make_move(try_find_move(moves3, 7, 63)); // DOWN b rook
        throwable_assert(initHash != b.currentState.zobristKey);
        b.print_state();

        moves3 = MoveList<false>(&b);
        b.make_move(try_find_move(moves3, 0, 56)); // DOWN w rook

        moves3 = MoveList<false>(&b);
        b.make_move(try_find_move(moves3, 63, 7)); // UP b rook

        b.print_state();
        throwable_assert(initHash == b.currentState.zobristKey);
        throwable_assert(b.is_draw());

        b.load_from_fen("7r/8/8/4k3/8/4K3/8/R7 w - - 0 1");
        uint64_t whiteHash = b.currentState.zobristKey;

        b.load_from_fen("7r/8/8/4k3/8/4K3/8/R7 b - - 0 1");
        uint64_t blackHash = b.currentState.zobristKey;

        throwable_assert(whiteHash != blackHash);
    }

    static Move try_find_move(MoveList<false>& moves, int from, int to){
        Move move;
        while ((move = moves.get()) != Move::none()){
            if(move.from() == from && move.to() == to) return move;
        }

        throwable_assert(false);
        return Move::none();
    }
};

#endif //SIGMOID_ZOBRIST_TESTS_HPP
