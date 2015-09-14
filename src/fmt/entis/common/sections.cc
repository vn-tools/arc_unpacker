#include "err.h"
#include "fmt/entis/common/sections.h"

using namespace au;
using namespace au::fmt::entis::common;

struct SectionReader::Priv final
{
    Priv(io::IO &io) : io(io)
    {
    }

    std::vector<Section> sections;
    io::IO &io;
};

SectionReader::SectionReader(io::IO &io) : p(new Priv(io))
{
    while (!io.eof())
    {
        Section section;
        section.name = io.read(8).str();
        section.size = io.read_u64_le();
        section.offset = io.tell();

        auto space_index = section.name.find_first_of('\x20');
        if (space_index != section.name.npos)
            section.name = section.name.substr(0, space_index);

        io.skip(section.size);
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
    auto sections = get_sections(name);
    if (!sections.size())
        throw err::CorruptDataError("Section " + name + " not found.");
    if (sections.size() > 1)
        throw std::logic_error("Section " + name + " occurs multiple times.");
    return sections[0];
}
