#include "compat/entry_point.h"
#include "arc_unpacker.h"
#include "log.h"

using namespace au;

ENTRY_POINT(
    try
    {
        ArcUnpacker arc_unpacker(arguments, AU_VERSION);

        if (!arc_unpacker.run())
            return -1;
        return 0;
    }
    catch (std::exception &e)
    {
        Log.err("Error: " + std::string(e.what()) + "\n");
        return 1;
    }
)
