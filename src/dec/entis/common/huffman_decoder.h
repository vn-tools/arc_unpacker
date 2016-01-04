#pragma once

#include "dec/entis/common/abstract_decoder.h"
#include "dec/entis/common/huffman_tree.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    int get_huffman_code(io::IBitReader &bit_reader, HuffmanTree &tree);
    int get_huffman_size(io::IBitReader &bit_reader, HuffmanTree &tree);

    class HuffmanDecoder final : public AbstractDecoder
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
