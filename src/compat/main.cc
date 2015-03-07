#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <memory>
#include "compat/main.h"

void ensure_utf8_locale()
{
    std::locale::global(boost::locale::generator().generate("en_US.UTF-8"));
    boost::filesystem::path::imbue(std::locale());
}

#ifdef _WIN32
    #include <Windows.h>
    #include "compat/winutf8.h"

    int run_with_args(
        int, const char **, std::function<int(std::vector<std::string>)> main)
    {
        ensure_utf8_locale();
        std::vector<std::string> arguments;
        int argc;
        LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        arguments.reserve(argc);
        for (int i = 0; i < argc; i ++)
        {
            arguments.push_back(wstring2string(argv[i]));
        }
        LocalFree(argv);
        return main(arguments);
    }
#else
    int run_with_args(
        int argc,
        const char **argv,
        std::function<int(std::vector<std::string>)> main)
    {
        ensure_utf8_locale();
        std::vector<std::string> arguments;
        arguments.reserve(argc);
        for (int i = 0; i < argc; i ++)
            arguments.push_back(std::string(argv[i]));
        return main(arguments);
    }
#endif
