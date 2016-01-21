#pragma once

#include <memory>
#include "io/base_bit_stream.h"
#include "io/base_byte_stream.h"
#include "types.h"

namespace au {
namespace dec {
namespace bgi {
namespace cbg {

    bstr read_decrypted_data(io::BaseByteStream &input_stream);

    using FreqTable = std::vector<u32>;

    struct NodeInfo final
    {
        bool valid;
        u32 frequency;
        u32 children[2];
    };

    struct Tree final
    {
        u32 get_leaf(io::BaseBitStream &bit_stream) const;

        NodeInfo &operator[](size_t);

        u32 size;
        std::vector<std::shared_ptr<NodeInfo>> nodes;
    };

    u32 read_variable_data(io::BaseByteStream &input_stream);

    FreqTable read_freq_table(
        io::BaseByteStream &input_stream, const size_t tree_size);

    Tree build_tree(const FreqTable &freq_table, const bool greedy);

} } } }
