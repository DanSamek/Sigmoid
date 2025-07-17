#ifndef SIGMOID_OLD_NNUE_HPP
#define SIGMOID_OLD_NNUE_HPP

#include <cassert>
#include <string>
#include <sstream>
#include <cstring>

#include "accumulator.hpp"
#include "../../constants.hpp"
#include "../../color.hpp"
#include "../../piece.hpp"
#include "../../3rdparty/incbin.h"


#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

INCBIN(net, STR(NET_BIN));

namespace Sigmoid{
    // Custom net 768 -> N -> 1 [no perspective].
    struct OldNNUE{
        std::array<OldAccumulator, STACK_SIZE_P1> stack;
        int stackIndex = 0;

        static inline std::array<int16_t, OLD_OUTPUT_SIZE> hiddenLayerBiases;
        static inline std::array<int16_t, OLD_HIDDEN_LAYER_SIZE> hiddenLayerWeights;

        static inline std::array<int16_t, OLD_HIDDEN_LAYER_SIZE> inputLayerBiases;
        static inline std::array<std::array<int16_t, OLD_HIDDEN_LAYER_SIZE>, OLD_NUM_FEATURES> inputLayerWeights;

        static constexpr int qa = 255;
        static constexpr int qb = 64;
        static constexpr int scale = 400;

        OldNNUE() {
            load();
            reset();
        }

        void pop(){
            stackIndex--;
        }

        void push(){
            stack[stackIndex + 1] = stack[stackIndex];
            stackIndex++;
        }

        void reset(){
            stackIndex = 0;
            stack[stackIndex].init(inputLayerBiases);
            load();
        }

        static int relu(int value) {
            return std::max(0, value);
        }

        void add(Color pieceColor, Piece piece, int square){
            assert(stackIndex >= 0);
            OldAccumulator* current_accumulator = &stack[stackIndex];
            int w_feature_index = get_index<WHITE>(pieceColor, piece, square);
            int b_feature_index = get_index<BLACK>(pieceColor, piece, square);

            current_accumulator->add<WHITE>(inputLayerWeights[w_feature_index]);
            current_accumulator->add<BLACK>(inputLayerWeights[b_feature_index]);
        }

        void sub(Color pieceColor, Piece piece, int square){
            assert(stackIndex >= 0);
            OldAccumulator* current_accumulator = &stack[stackIndex];
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
        int16_t eval() {
            assert(stackIndex >= 0);
            const auto our_accumulator = stack[stackIndex].get<color>();

            int eval = hiddenLayerBiases[0];
            for (int i = 0 ; i < OLD_HIDDEN_LAYER_SIZE; i++)
                eval += hiddenLayerWeights[i] * relu(our_accumulator[i]);

            eval *= scale;
            eval /= qa * qb;
            return eval;
        }

        template<Color perspective>
        int get_index(Color pieceColor, Piece piece, int square){
            int color_index = (pieceColor == perspective) ? 0 : 1;
            int piece_index = piece;
            int square_index = perspective == WHITE ? square : square ^ 56;

            int result_index = color_index * 384 + piece_index * 64 + square_index;
            assert(result_index >= 0 && result_index <= 767);
            return result_index;
        }

        template<typename T>
        T read_number(int& index, const unsigned char*& ptr){
            T value;
            std::memcpy(&value, ptr, sizeof(T));
            index += sizeof(T);
            ptr += sizeof(T);
            return value;
        }

        static inline bool loaded = false;

        void load() {
            if (loaded)
                return;

            const unsigned char* ptr = gnetData;
            int index = 0;
            assert(index < gnetSize);
            for(int i = 0; i < OLD_NUM_FEATURES; i++)
                for(int x = 0; x < OLD_HIDDEN_LAYER_SIZE; x++)
                    inputLayerWeights[i][x] = read_number<int>(index, ptr);

            assert(index < gnetSize);
            for(int i = 0; i < OLD_HIDDEN_LAYER_SIZE; i++) {
                inputLayerBiases[i] = read_number<int>(index, ptr);
                hiddenLayerWeights[i] = read_number<int>(index, ptr);
            }

            assert(index < gnetSize);
            hiddenLayerBiases[0] = read_number<int>(index, ptr);
            assert(index == gnetSize);
            loaded = true;
        }
    };
}

#endif //SIGMOID_OLD_NNUE_HPP
