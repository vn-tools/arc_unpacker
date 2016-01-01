#include "dec/entis/common/sections.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis::common;

struct SectionReader::Priv final
{
    Priv(io::IStream &input_stream) : input_stream(input_stream)
    {
    }

    std::vector<Section> sections;
    io::IStream &input_stream;
};

SectionReader::SectionReader(io::IStream &input_stream)
    : p(new Priv(input_stream))
{
    while (!input_stream.eof())
    {
        Section section;
        section.name = input_stream.read(8).str();
        section.size = input_stream.read_u64_le();
        section.offset = input_stream.tell();

        const auto space_index = section.name.find_first_of('\x20');
        if (space_index != section.name.npos)
            section.name = section.name.substr(0, space_index);

        input_stream.skip(section.size);
        p->sections.push_back(section);
    }
}

SectionReader::~SectionReader()
{
}

std::vector<Section> SectionReader::get_sections() const
{
    return p->sections;
}

std::vector<Section> SectionReader::get_sections(const std::string &name) const
{
    std::vector<Section> sections;
    for (auto &section : p->sections)
        if (section.name == name)
            sections.push_back(section);
    return sections;
}

Section SectionReader::get_section(const std::string &name) const
{
    const auto sections = get_sections(name);
    if (!sections.size())
        throw err::CorruptDataError("Section " + name + " not found.");
    if (sections.size() > 1)
        throw std::logic_error("Section " + name + " occurs multiple times.");
    return sections.at(0);
}
