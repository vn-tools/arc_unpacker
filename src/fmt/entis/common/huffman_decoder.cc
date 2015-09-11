#include "fmt/entis/common/huffman_decoder.h"
#include "fmt/entis/common/gamma_decoder.h"
#include "io/bit_reader.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::entis::common;

struct HuffmanDecoder::Priv
{
    Priv(io::BitReader &bit_reader);
    void decode(u8 *output, size_t output_size);

    int get_huffman_size(HuffmanTree &tree);
    int get_huffman_code(HuffmanTree &tree);

    std::vector<std::shared_ptr<HuffmanTree>> huffman_trees;
    std::shared_ptr<HuffmanTree> last_huffman_tree;
    io::BitReader &bit_reader;
    size_t available_size;
};

HuffmanDecoder::Priv::Priv(io::BitReader &bit_reader) : bit_reader(bit_reader)
{
    for (auto i : util::range(0x101))
    {
        huffman_trees.push_back(
            std::shared_ptr<HuffmanTree>(new HuffmanTree));
    }
    last_huffman_tree = huffman_trees[0];
    available_size = 0;
}

void HuffmanDecoder::Priv::decode(u8 *output, size_t output_size)
{
    auto tree = last_huffman_tree;

    u8 *output_ptr = output;
    u8 *output_end = output + output_size;

    if ( available_size > 0 )
    {
        auto size = std::min<size_t>(available_size, output_size);
        available_size -= size;
        while (size-- && output_ptr < output_end)
            *output_ptr++ = 0;
    }

    while (output_ptr < output_end)
    {
        int symbol = get_huffman_code(*tree);
        if (symbol == HuffmanFlags::Escape)
            break;
        *output_ptr++ = symbol;

        if (!symbol)
        {
            int size = get_huffman_size(*huffman_trees[0x100]);
            if (size == HuffmanFlags::Escape)
                break;
            if (--size)
            {
                available_size = size;
                if (output_ptr + size > output_end)
                    size = output_end - output_ptr;
                available_size -= size;
                while (size-- && output_ptr < output_end)
                    *output_ptr++ = 0;
            }
        }
        tree = huffman_trees[symbol & 0xFF];
    }
    last_huffman_tree = tree;
}

int HuffmanDecoder::Priv::get_huffman_size(HuffmanTree &tree)
{
    if (tree.escape != HuffmanNodes::Null)
    {
        int entry = HuffmanNodes::Root;
        int child = tree.nodes[HuffmanNodes::Root].code;
        do
        {
            if (bit_reader.eof())
                return HuffmanFlags::Escape;
            entry = child + bit_reader.get(1);
            child = tree.nodes[entry].code;
        }
        while (!(child & HuffmanFlags::Code));

        tree.increase_occurrences(entry);
        int code = child & ~HuffmanFlags::Code;
        if (code != HuffmanFlags::Escape)
            return code;
    }
    int code = get_gamma_code(bit_reader);
    if (code == -1)
        return HuffmanFlags::Escape;
    tree.add_new_entry(code);
    return code;
}

int HuffmanDecoder::Priv::get_huffman_code(HuffmanTree &tree)
{
    if (tree.escape != HuffmanNodes::Null)
    {
        int entry = HuffmanNodes::Root;
        int child = tree.nodes[HuffmanNodes::Root].code;
        while (!(child & HuffmanFlags::Code))
        {
            if (bit_reader.eof())
                return HuffmanFlags::Escape;
            entry = child + bit_reader.get(1);
            child = tree.nodes[entry].code;
        }

        tree.increase_occurrences(entry);
        int code = child & ~HuffmanFlags::Code;
        if (code != HuffmanFlags::Escape)
            return code;
    }
    if (bit_reader.eof())
        return HuffmanFlags::Escape;
    int code = bit_reader.get(8);
    tree.add_new_entry(code);
    return code;
}

HuffmanDecoder::HuffmanDecoder(const bstr &data)
    : Decoder(data), p(new Priv(bit_reader))
{
}

HuffmanDecoder::~HuffmanDecoder()
{
}

int HuffmanDecoder::get_huffman_code(HuffmanTree &tree)
{
    return p->get_huffman_code(tree);
}

int HuffmanDecoder::get_huffman_size(HuffmanTree &tree)
{
    return p->get_huffman_size(tree);
}

void HuffmanDecoder::decode(u8 *output, size_t output_size)
{
    p->decode(output, output_size);
}
