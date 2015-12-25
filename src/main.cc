#include "cli_facade.h"
#include "entry_point.h"
#include "io/program_path.h"
#include "log.h"

using namespace au;

ENTRY_POINT(
    try
    {
        io::set_program_path_from_arg(arguments[0]);
        arguments.erase(arguments.begin());
        CliFacade cli_facade(arguments);
        return cli_facade.run();
    }
    catch (std::exception &e)
    {
        Log.err("Error: " + std::string(e.what()) + "\n");
        return 1;
    }
)
