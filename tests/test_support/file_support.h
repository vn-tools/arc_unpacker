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

#pragma once

#include <memory>
#include "io/file.h"

namespace au {
namespace tests {

    std::unique_ptr<io::File> stub_file(
        const std::string &path, const bstr &data);

    std::unique_ptr<io::File> file_from_path(
        const io::path &path, const std::string &custom_path = "");

    std::unique_ptr<io::File> zlib_file_from_path(
        const io::path &path, const std::string &custom_path = "");

    void compare_files(
        const io::File &actual_file,
        const io::File &expected_file,
        const bool compare_file_paths);

    void compare_files(
        const std::vector<std::shared_ptr<io::File>> &actual_files,
        const std::vector<std::shared_ptr<io::File>> &expected_files,
        const bool compare_file_paths);

    void dump_file(io::File &input_file, const io::path &path);

} }
