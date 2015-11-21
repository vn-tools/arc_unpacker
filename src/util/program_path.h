#pragma once

#include <string>
#include "io/path.h"

namespace au {
namespace util {

    void set_program_path_from_arg(const std::string &arg);
    io::path get_program_path();
    io::path get_extra_dir_path();

} }
