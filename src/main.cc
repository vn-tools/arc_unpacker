// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

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
