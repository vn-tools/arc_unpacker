#ifndef AU_FMT_BGI_CBG_CBG_COMMON_H
#define AU_FMT_BGI_CBG_CBG_COMMON_H
#include <memory>
#include "io/io.h"
#include "io/bit_reader.h"
#include "types.h"

namespace au {
namespace fmt {
namespace bgi {
namespace cbg {

    bstr read_decrypted_data(io::IO &io);

    using FreqTable = std::vector<u32>;

    struct NodeInfo
    {
        bool valid;
        u32 frequency;
        u32 children[2];
    };

    struct Tree
    {
        u32 root;
        u32 size;
        std::vector<std::shared_ptr<NodeInfo>> nodes;
        NodeInfo &operator[](size_t);
        u32 get_leaf(io::BitReader &bit_reader) const;
    };

    u32 read_variable_data(io::IO &input_io);
    FreqTable read_freq_table(io::IO &input_io, size_t tree_size);
    Tree build_tree(const FreqTable &freq_table, bool greedy);

} } } }

#endif
