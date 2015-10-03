#pragma once

#include <memory>
#include "io/io.h"

namespace au {
namespace fmt {
namespace wild_bug {

    struct WpxSection final
    {
        u32 data_format;
        u32 offset;
        u32 size_orig;
        u32 size_comp;
    };

    class WpxDecoder final
    {
    public:
        WpxDecoder(io::IO &io);
        ~WpxDecoder();

        std::string get_tag() const;
        bool has_section(u8 section_id) const;
        bstr read_plain_section(u8 section_id);
        bstr read_compressed_section(
            u8 section_id, u8 quant_size, const std::array<size_t, 8> &offsets);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
