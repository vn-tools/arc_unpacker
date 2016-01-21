#pragma once

#include <memory>
#include "io/base_byte_stream.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    struct Section final
    {
        std::string name;
        size_t size;
        size_t base_offset;
        size_t data_offset;
    };

    class SectionReader final
    {
    public:
        SectionReader(io::BaseByteStream &input_stream);
        ~SectionReader();
        Section get_section(const std::string &name) const;
        bool has_section(const std::string &name) const;
        std::vector<Section> get_sections() const;
        std::vector<Section> get_sections(const std::string &name) const;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
