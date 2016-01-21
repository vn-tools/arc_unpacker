#pragma once

#include "dec/entis/common/base_decoder.h"
#include "dec/entis/common/huffman_tree.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    int get_huffman_code(io::BaseBitStream &bit_stream, HuffmanTree &tree);
    int get_huffman_size(io::BaseBitStream &bit_stream, HuffmanTree &tree);

    class HuffmanDecoder final : public BaseDecoder
    {
    public:
        HuffmanDecoder();
        ~HuffmanDecoder();

        void reset() override;
        void decode(u8 *output, const size_t output_size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
