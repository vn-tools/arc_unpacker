#pragma once

#include <memory>
#include "io/bit_reader.h"
#include "io/stream.h"
#include "types.h"

namespace au {
namespace fmt {
namespace bgi {
namespace cbg {

    bstr read_decrypted_data(io::Stream &input_stream);

    using FreqTable = std::vector<u32>;

    struct NodeInfo final
    {
        bool valid;
        u32 frequency;
        u32 children[2];
    };

    struct Tree final
    {
        u32 get_leaf(io::BitReader &bit_reader) const;

        NodeInfo &operator[](size_t);

        u32 size;
        std::vector<std::shared_ptr<NodeInfo>> nodes;
    };

    u32 read_variable_data(io::Stream &input_stream);
    FreqTable read_freq_table(io::Stream &input_stream, size_t tree_size);
    Tree build_tree(const FreqTable &freq_table, bool greedy);

} } } }
