// DSC file wrapper
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC
//
// Known games:
// - Higurashi No Naku Koro Ni
// - Go! Go! Nippon! ~My First Trip to Japan~

#include <algorithm>
#include "fmt/bgi/common.h"
#include "fmt/bgi/dsc_converter.h"
#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"
#include "util/require.h"

using namespace au;
using namespace au::fmt::bgi;

namespace
{
    struct NodeInfo
    {
        bool has_children;
        bool look_behind;
        u8 value;
        u32 children[2];
    };

    using NodeList = std::vector<std::unique_ptr<NodeInfo>>;
}

static const bstr magic = "DSC FORMAT 1.00\x00"_b;

static int is_image(const bstr &input)
{
    io::BufferedIO input_io(input);
    auto width = input_io.read_u16_le();
    auto height = input_io.read_u16_le();
    auto bpp = input_io.read_u8();
    auto zeros = input_io.read(11);
    for (auto i : util::range(zeros.size()))
        if (zeros[i])
            return false;
    return width && height && (bpp == 8 || bpp == 24 || bpp == 32);
}

static NodeList get_nodes(io::IO &io, u32 key)
{
    NodeList nodes;
    for (auto i : util::range(1024))
    {
        std::unique_ptr<NodeInfo> node_info(new NodeInfo);
        node_info->has_children = false;
        node_info->value = 0;
        nodes.push_back(std::move(node_info));
    }

    std::vector<u32> arr0;
    for (auto n : util::range(512))
    {
        u8 tmp = io.read_u8() - get_and_update_key(key);
        if (tmp)
            arr0.push_back((tmp << 16) + n);
    }

    std::sort(arr0.begin(), arr0.end());

    size_t arr0_pos;
    u32 n = 0, unk0 = 0x200, unk1 = 1, node_index = 1;
    u32 arr1[1024] = {0};
    u32 *node_ptr = arr1;
    for (arr0_pos = 0; arr0_pos < arr0.size(); n++)
    {
        u32 *arr1_ptr = &arr1[unk0];
        u32 *arr1_old_ptr = arr1_ptr;
        u32 group_count = 0;

        while (true)
        {
            u32 c = arr0_pos < arr0.size() ? arr0[arr0_pos] : 0;
            if (n != (c >> 16))
                break;
            nodes[*node_ptr]->has_children = false;
            nodes[*node_ptr]->look_behind = arr0[arr0_pos] & 0x100;
            nodes[*node_ptr]->value = arr0[arr0_pos] & 0xFF;
            arr0_pos++;
            node_ptr++;
            group_count++;
        }

        u32 unk3 = 2 * (unk1 - group_count);
        if (group_count < unk1)
        {
            unk1 = unk1 - group_count;
            for (auto i : util::range(unk1))
            {
                nodes[*node_ptr]->has_children = true;
                for (auto j : util::range(2))
                    *arr1_ptr++ = nodes[*node_ptr]->children[j] = node_index++;
                node_ptr++;
            }
        }
        unk1 = unk3;
        node_ptr = arr1_old_ptr;
        unk0 ^= 0x200;
    }
    return nodes;
}

static bstr decompress(io::IO &io, const NodeList &nodes, size_t output_size)
{
    bstr output;
    output.resize(output_size);
    u8 *output_ptr = output.get<u8>();
    const u8 *output_start = output_ptr;
    const u8 *output_end = output_ptr + output.size();
    io::BitReader bit_reader(io.read_to_eof());

    u32 bits = 0, bit_count = 0;
    while (output_ptr < output_end)
    {
        u32 node_index = 0;
        while (nodes[node_index]->has_children)
            node_index = nodes[node_index]->children[bit_reader.get(1)];

        if (nodes[node_index]->look_behind)
        {
            auto offset = bit_reader.get(12);
            u32 repetitions = nodes[node_index]->value + 2;
            u8 *look_behind = output_ptr - offset - 2;
            util::require(look_behind >= output_start);
            util::require(look_behind + repetitions < output_end);
            while (repetitions--)
                *output_ptr++ = *look_behind++;
        }
        else
        {
            *output_ptr++ = nodes[node_index]->value;
        }
    }

    return output;
}

bool DscConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> DscConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto key = file.io.read_u32_le();
    auto output_size = file.io.read_u32_le();
    file.io.skip(8);

    auto nodes = get_nodes(file.io, key);
    auto data = decompress(file.io, std::move(nodes), output_size);

    if (is_image(data))
    {
        io::BufferedIO data_io(data);
        auto width = data_io.read_u16_le();
        auto height = data_io.read_u16_le();
        auto bpp = data_io.read_u8();
        data_io.skip(11);

        pix::Format fmt;
        switch (bpp)
        {
            case 8:
                fmt = pix::Format::Gray8;
                break;
            case 24:
                fmt = pix::Format::BGR888;
                break;
            case 32:
                fmt = pix::Format::BGRA8888;
                break;
            default:
                util::fail(util::format("Unsupported bit depth: %d", bpp));
        }
        pix::Grid pixels(width, height, data_io.read_to_eof(), fmt);
        return util::Image::from_pixels(pixels)->create_file(file.name);
    }

    std::unique_ptr<File> output_file(new File);
    output_file->io.write(data);
    output_file->name = file.name;
    if (!output_file->has_extension())
        output_file->change_extension("dat");
    output_file->guess_extension();
    return output_file;
}
