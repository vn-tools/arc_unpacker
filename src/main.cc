#include <iostream>
#include "compat/entry_point.h"
#include "arc_unpacker.h"

ENTRY_POINT(
    try
    {
        ArgParser arg_parser;
        arg_parser.parse(arguments);
        ArcUnpacker arc_unpacker(arg_parser);

        if (!arc_unpacker.run())
            return -1;
        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
)
