#ifndef SIGMOID_MAILBOX_TESTS_HPP
#define SIGMOID_MAILBOX_TESTS_HPP

#include <array>

#include "test.hpp"
#include "../mailbox.hpp"

using namespace Sigmoid;

struct MailboxTests : public Test{

    std::string test_name() const override{
        return "MailboxTests";
    }

    void run() const override{
        MailBox mb;
        assert(mb.at(0) == Piece::NONE);
        assert(mb.at(63) == Piece::NONE);

        mb.set_piece(0, Piece::KING);
        mb.set_piece(63, Piece::KING);

        assert(mb.at(0) == Piece::KING);
        assert(mb.at(1) == Piece::NONE);

        assert(mb.at(62) == Piece::NONE);
        assert(mb.at(63) == Piece::KING);

        mb.clear();

        std::array<Piece, 64> expected_pieces;
        for (int square = 0; square < 64; square++){
            Piece random_piece = Piece(rand() % 6);
            mb.set_piece(square, random_piece);

            assert(mb.at(square) == random_piece);
            expected_pieces[square] = random_piece;
        }

        for (int square = 0; square < 64; square++){
            assert(mb.at(square) == expected_pieces[square]);
        }

    }
};

#endif //SIGMOID_MAILBOX_TESTS_HPP
