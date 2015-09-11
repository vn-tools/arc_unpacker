#pragma once

#include "fmt/entis/common/decoder.h"
#include "fmt/entis/common/huffman_tree.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    class HuffmanDecoder final : public Decoder
    {
    public:
        HuffmanDecoder(const bstr &data);
        ~HuffmanDecoder();

        virtual void decode(u8 *output, size_t output_size) override;
        int get_huffman_code(HuffmanTree &tree);
        int get_huffman_size(HuffmanTree &tree);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
