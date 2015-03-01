#include <functional>
#include "factory/converter_factory.h"
#include "formats/bgi/cbg_converter.h"
#include "formats/bgi/sound_converter.h"
#include "formats/french_bread/ex3_converter.h"
#include "formats/ivory/prs_converter.h"
#include "formats/key/g00_converter.h"
#include "formats/key/nwa_converter.h"
#include "formats/kirikiri/tlg_converter.h"
#include "formats/liarsoft/wcg_converter.h"
#include "formats/lizsoft/sotes_converter.h"
#include "formats/nscripter/spb_converter.h"
#include "formats/nsystem/mgd_converter.h"
#include "formats/qlie/dpng_converter.h"
#include "formats/rpgmaker/xyz_converter.h"
#include "formats/yukascript/ykg_converter.h"
using namespace Formats;

struct ConverterFactory::Internals
{
    std::vector<std::pair<std::string, std::function<Converter*()>>> formats;

    void add(std::string format, std::function<Converter*()> creator)
    {
        formats.push_back(
            std::pair<std::string, std::function<Converter*()>>(
                format, creator));
    }
};

ConverterFactory::ConverterFactory() : internals(new Internals)
{
    internals->add("wcg", []() { return new LiarSoft::WcgConverter(); });
    internals->add("dpng", []() { return new QLiE::DpngConverter(); });
    internals->add("bgi-sound", []() { return new Bgi::SoundConverter(); });
    internals->add("ex3", []() { return new FrenchBread::Ex3Converter(); });
    internals->add("tlg", []() { return new Kirikiri::TlgConverter(); });
    internals->add("ykg", []() { return new YukaScript::YkgConverter(); });
    internals->add("cbg", []() { return new Bgi::CbgConverter(); });
    internals->add("xyz", []() { return new RpgMaker::XyzConverter(); });
    internals->add("mgd", []() { return new NSystem::MgdConverter(); });
    internals->add("g00", []() { return new Key::G00Converter(); });
    internals->add("sotes", []() { return new Lizsoft::SotesConverter(); });
    internals->add("nwa", []() { return new Key::NwaConverter(); });
    internals->add("prs", []() { return new Ivory::PrsConverter(); });
    internals->add("spb", []() { return new NScripter::SpbConverter(); });
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
