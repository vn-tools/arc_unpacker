#include "arc_unpacker.h"
#include "entry_point.h"
#include "io/program_path.h"
#include "log.h"

using namespace au;

ENTRY_POINT(
    try
    {
        io::set_program_path_from_arg(arguments[0]);
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
