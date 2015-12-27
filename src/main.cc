#include "entry_point.h"
#include "flow/cli_facade.h"
#include "io/program_path.h"
#include "logger.h"

using namespace au;

ENTRY_POINT(
    Logger logger;
    try
    {
        io::set_program_path_from_arg(arguments[0]);
        arguments.erase(arguments.begin());
        flow::CliFacade cli_facade(logger, arguments);
        return cli_facade.run();
    }
    catch (const std::exception &e)
    {
        logger.err("Error: " + std::string(e.what()) + "\n");
        return 1;
    }
)
