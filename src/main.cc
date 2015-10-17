#include "compat/entry_point.h"
#include "arc_unpacker.h"
#include "log.h"
#include "util/program_path.h"

using namespace au;

ENTRY_POINT(
    try
    {
        util::set_program_path_from_arg(arguments[0]);
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
