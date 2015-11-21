#include "util/version_recognizer.h"
#include <map>
#include "err.h"

using namespace au;
using namespace au::util;

struct VersionRecognizer::Priv final
{
    std::map<int, std::function<bool(io::File &)>> funcs;
};

VersionRecognizer::VersionRecognizer() : p(new Priv())
{
}

VersionRecognizer::~VersionRecognizer()
{
}

void VersionRecognizer::add_recognizer(
    const int version, std::function<bool(io::File &)> func)
{
    p->funcs[version] = func;
}

int VersionRecognizer::tell_version(io::File &file) const
{
    const auto old_pos = file.stream.tell();
    for (const auto it : p->funcs)
    {
        try
        {
            const bool result = it.second(file);
            file.stream.seek(old_pos);
            if (result)
                return it.first;
        }
        catch (...)
        {
            continue;
        }
    }
    throw err::RecognitionError();
}
