#include <boost/filesystem.hpp>
#include "arc_unpacker.h"
#include "test_support/eassert.h"
#include "test_support/suppress_output.h"

void test_converter()
{
    ArgParser arg_parser;
    arg_parser.parse({
        "path-to-self",
        "./tests/formats/key/files/AYU_03.g00",
        "--fmt=g00" });
    ArcUnpacker arc_unpacker(arg_parser);
    arc_unpacker.run();

    eassert(boost::filesystem::is_regular_file("./AYU_03.png"));
    boost::filesystem::remove("./AYU_03.png");
}

void test_archive()
{
    ArgParser arg_parser;
    arg_parser.parse({
        "path-to-self",
        "./tests/formats/kirikiri/files/xp3-v2.xp3",
        "--fmt=xp3",
        "--plugin=noop" });
    ArcUnpacker arc_unpacker(arg_parser);
    arc_unpacker.run();

    eassert(boost::filesystem::is_directory("./xp3-v2~.xp3"));
    eassert(boost::filesystem::is_regular_file("./xp3-v2~.xp3/abc.txt"));
    eassert(boost::filesystem::is_regular_file("./xp3-v2~.xp3/abc2.txt"));
    boost::filesystem::remove("./xp3-v2~.xp3/abc2.txt");
    boost::filesystem::remove("./xp3-v2~.xp3/abc.txt");
    boost::filesystem::remove("./xp3-v2~.xp3");
}

int main(void)
{
    suppress_output([&]()
    {
        test_converter();
        test_archive();
    });
}
