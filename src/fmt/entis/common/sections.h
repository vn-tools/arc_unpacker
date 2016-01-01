#pragma once

#include <memory>
#include "io/istream.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    struct Section final
    {
        std::string name;
        size_t size;
        size_t offset;
    };

    class SectionReader final
    {
    public:
        SectionReader(io::IStream &input_stream);
        ~SectionReader();
        Section get_section(const std::string &name) const;
        std::vector<Section> get_sections() const;
        std::vector<Section> get_sections(const std::string &name) const;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
