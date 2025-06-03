#ifndef SIGMOID_NNUE_HPP
#define SIGMOID_NNUE_HPP

#include <cassert>
#include <string>
#include <fstream>

#include "accumulator.hpp"
#include "../constants.hpp"
#include "../color.hpp"
#include "../piece.hpp"

namespace Sigmoid{

    struct NNUE{
        std::array<Accumulator, STACK_SIZE_P1> stack;
        int index = 0;

        static inline std::array<int16_t, OUTPUT_SIZE> hiddenLayerBiases;
        static inline std::array<int16_t, 2 * HIDDEN_LAYER_SIZE> hiddenLayerWeights;

        static inline std::array<int16_t, HIDDEN_LAYER_SIZE> inputLayerBiases;
        static inline std::array<std::array<int16_t, HIDDEN_LAYER_SIZE>, NUM_FEATURES> inputLayerWeights;

        static constexpr int qa = 255;
        static constexpr int qb = 64;
        static constexpr int scale = 400;

        std::string NET_PATH = "../src/nnue/singularity_v2_4-40.bin";

        void pop(){
            index--;
        }

        void push(){
            stack[index + 1] = stack[index];
            index++;
        }

        void reset(){
            index = 0;
            stack[index].init(inputLayerBiases);
            load_from_file();
        }

        int16_t crelu(int value) {
            return std::clamp(value, 0, qa);
        }

        void add(Color pieceColor, Piece piece, int square){
            Accumulator* current_accumulator = &stack[index];
            int w_feature_index = get_index<WHITE>(pieceColor, piece, square);
            int b_feature_index = get_index<BLACK>(pieceColor, piece, square);

            current_accumulator->add<WHITE>(inputLayerWeights[w_feature_index]);
            current_accumulator->add<BLACK>(inputLayerWeights[b_feature_index]);
        }

        void sub(Color pieceColor, Piece piece, int square){
            Accumulator* current_accumulator = &stack[index];
            int w_feature_index = get_index<WHITE>(pieceColor, piece, square);
            int b_feature_index = get_index<BLACK>(pieceColor, piece, square);

            current_accumulator->sub<WHITE>(inputLayerWeights[w_feature_index]);
            current_accumulator->sub<BLACK>(inputLayerWeights[b_feature_index]);
        }

        void move_piece(Color pieceColor, Piece piece, int from, int to){
            sub(pieceColor, piece, from);
            add(pieceColor, piece, to);
        }

        template<Color color>
        int16_t eval(){
            const auto our_accumulator = stack[index].get<color>();
            const auto opp_accumulator = stack[index].get<~color>();

            int eval = hiddenLayerBiases[0];
            for (int i = 0 ; i < HIDDEN_LAYER_SIZE; i++){
                eval += hiddenLayerWeights[i] * crelu(our_accumulator[i]);
            }

            for (int i = 0; i < HIDDEN_LAYER_SIZE; i++){
                eval += hiddenLayerWeights[i + HIDDEN_LAYER_SIZE] * crelu(opp_accumulator[i]);
            }

            eval *= scale;
            eval /= qa * qb;

            return eval;
        }

        template<Color perspective>
        int16_t get_index(Color pieceColor, Piece piece, int square){
            int color_index = (pieceColor == perspective) ? 0 : 1;
            int piece_index = piece;
            int square_index = perspective == WHITE ? square ^ 56: square;

            int result_index = color_index * 384 + piece_index * 64 + square_index;
            assert(result_index >= 0 && result_index <= 767);
            return result_index;
        }

        template<typename T>
        T read_number(std::ifstream & stream){
            T value;
            stream.read(reinterpret_cast<char*>(&value), sizeof(value));
            return value;
        }


        static inline bool loaded = false;

        void load_from_file() {
            if (loaded)
                return;

            auto stream = std::ifstream(NET_PATH, std::ios::binary);
            if(!stream.is_open())
                throw std::invalid_argument("File cannot be opened.");

            for(int i = 0; i < NUM_FEATURES; i++)
                for(int x = 0; x < HIDDEN_LAYER_SIZE; x++)
                    inputLayerWeights[i][x] = read_number<int16_t>(stream);

            assert(!stream.eof());
            for(int i = 0; i < HIDDEN_LAYER_SIZE; i++)
                inputLayerBiases[i] = read_number<int16_t>(stream);


            assert(!stream.eof());
            for(int i = 0; i < HIDDEN_LAYER_SIZE * 2; i++)
                hiddenLayerWeights[i] = read_number<int16_t>(stream);

            assert(!stream.eof());
            hiddenLayerBiases[0] = read_number<int16_t>(stream);
            loaded = true;
        }
    };
}

#endif //SIGMOID_NNUE_HPP
