#pragma once

#include "types.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    enum HuffmanFlags
    {
        Code = 0x80000000,
        Escape = 0x7FFFFFFF,
    };

    enum HuffmanNodes
    {
        Null = 0x8000,
        Root = 0x200,
    };

    struct HuffmanNode final
    {
        HuffmanNode();

        u16 weight;
        u16 parent;
        u32 code;
    };

    struct HuffmanTree final
    {
        HuffmanTree();
        void increase_occurrences(int entry);
        void recount_occurrences(int parent);
        void normalize(int entry);
        void add_new_entry(int new_code);
        void half_and_rebuild();

        HuffmanNode nodes[0x201];
        int sym_lookup[0x100];
        int escape;
        int tree_pointer;
    };

} } } }
