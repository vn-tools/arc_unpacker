#include <iostream>
#include "compat/entry_point.h"
#include "arc_unpacker.h"

using namespace au;

ENTRY_POINT(
    try
    {
        ArgParser arg_parser;
        arg_parser.parse(arguments);
        ArcUnpacker arc_unpacker(arg_parser, AU_VERSION);

        if (!arc_unpacker.run())
            return -1;
        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
)
