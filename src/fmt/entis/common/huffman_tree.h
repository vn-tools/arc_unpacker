#pragma once

#include "types.h"

namespace au {
namespace fmt {
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
        u16 weight;
        u16 parent;
        u32 code;
        HuffmanNode();
    };

    struct HuffmanTree final
    {
        HuffmanTree();
        void increase_occurrences(int iEntry);
        void recount_occurrences(int iParent);
        void normalize(int iEntry);
        void add_new_entry(int nNewCode);
        void half_and_rebuild();

        HuffmanNode nodes[0x201];
        int sym_lookup[0x100];
        int escape;
        int tree_pointer;
    };

} } } }
