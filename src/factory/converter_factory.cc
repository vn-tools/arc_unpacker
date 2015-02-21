#include <functional>
#include "factory/converter_factory.h"
#include "formats/gfx/cbg_converter.h"
#include "formats/gfx/dpng_converter.h"
#include "formats/gfx/ex3_converter.h"
#include "formats/gfx/g00_converter.h"
#include "formats/gfx/mgd_converter.h"
#include "formats/gfx/prs_converter.h"
#include "formats/gfx/sotes_converter.h"
#include "formats/gfx/spb_converter.h"
#include "formats/gfx/tlg_converter.h"
#include "formats/gfx/wcg_converter.h"
#include "formats/gfx/xyz_converter.h"
#include "formats/gfx/ykg_converter.h"
#include "formats/sfx/bgi_converter.h"
#include "formats/sfx/nwa_converter.h"

struct ConverterFactory::Internals
{
    std::vector<std::pair<std::string, std::function<Converter*()>>> formats;

    void add_format(std::string format, std::function<Converter*()> creator)
    {
        formats.push_back(
            std::pair<std::string, std::function<Converter*()>>(
                format, creator));
    }
};

ConverterFactory::ConverterFactory() : internals(new Internals)
{
    internals->add_format("wcg", []() { return new WcgConverter(); });
    internals->add_format("dpng", []() { return new DpngConverter(); });
    internals->add_format("bgi-sound", []() { return new BgiConverter(); });
    internals->add_format("ex3", []() { return new Ex3Converter(); });
    internals->add_format("tlg", []() { return new TlgConverter(); });
    internals->add_format("ykg", []() { return new YkgConverter(); });
    internals->add_format("cbg", []() { return new CbgConverter(); });
    internals->add_format("xyz", []() { return new XyzConverter(); });
    internals->add_format("mgd", []() { return new MgdConverter(); });
    internals->add_format("g00", []() { return new G00Converter(); });
    internals->add_format("sotes", []() { return new SotesConverter(); });
    internals->add_format("nwa", []() { return new NwaConverter(); });
    internals->add_format("prs", []() { return new PrsConverter(); });
    internals->add_format("spb", []() { return new SpbConverter(); });
}

ConverterFactory::~ConverterFactory()
{
}

const std::vector<std::string> ConverterFactory::get_formats() const
{
    std::vector<std::string> formats;
    for (auto &p : internals->formats)
        formats.push_back(p.first);
    return formats;
}

Converter *ConverterFactory::create_converter(const std::string format) const
{
    for (auto &p : internals->formats)
        if (p.first == format)
            return p.second();
    throw std::runtime_error("Invalid converter format: " + format);
}
