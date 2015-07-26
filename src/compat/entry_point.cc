#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/locale.hpp>
#include "compat/entry_point.h"
#include "util/encoding.h"

using namespace au;

std::vector<std::string> get_arguments(int argc, const char **argv)
{
    std::vector<std::string> arguments;
    arguments.reserve(argc);
    for (int i = 0; i < argc; i++)
        arguments.push_back(std::string(argv[i]));
    return arguments;
}

std::vector<std::string> get_arguments(int argc, const wchar_t **argv)
{
    std::vector<std::string> arguments;
    arguments.reserve(argc);
    for (int i = 0; i < argc; i++)
    {
        auto str = std::string(
            reinterpret_cast<const char*>(argv[i]), wcslen(argv[i]) * 2);
        arguments.push_back(util::convert_encoding(str, "utf-16le", "utf-8"));
    }
    return arguments;
}

void init_fs_utf8()
{
    std::locale loc(
        std::locale(),
        new boost::filesystem::detail::utf8_codecvt_facet);
    boost::filesystem::path::imbue(loc);
}
