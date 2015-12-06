#pragma once

#include <string>
#include "io/path.h"

namespace au {
namespace io {

    void set_program_path_from_arg(const std::string &arg);
    path get_program_path();
    path get_etc_dir_path();

} }
