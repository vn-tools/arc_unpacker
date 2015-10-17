#pragma once

#include <boost/filesystem/path.hpp>
#include <string>

namespace au {
namespace util {

    void set_program_path_from_arg(const std::string &arg);
    boost::filesystem::path get_program_path();
    boost::filesystem::path get_extra_dir_path();

} }
