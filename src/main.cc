#include "compat/entry_point.h"
#include "arc_unpacker.h"
#include "log.h"

using namespace au;

ENTRY_POINT(
    try
    {
        arguments.erase(arguments.begin());
        ArcUnpacker arc_unpacker(arguments, AU_VERSION);

        return arc_unpacker.run();
    }
    catch (std::exception &e)
    {
        Log.err("Error: " + std::string(e.what()) + "\n");
        return 1;
    }
)
