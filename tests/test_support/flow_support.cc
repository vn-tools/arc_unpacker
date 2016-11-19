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

#include "test_support/flow_support.h"
#include "flow/file_saver_callback.h"
#include "flow/parallel_unpacker.h"

using namespace au;

std::vector<std::shared_ptr<io::File>> tests::flow_unpack(
    const dec::Registry &registry,
    const bool enable_nested_decoding,
    io::File &input_file)
{
    Logger dummy_logger;
    dummy_logger.mute();

    std::vector<std::shared_ptr<io::File>> saved_files;
    const flow::FileSaverCallback file_saver(
        [&](std::shared_ptr<io::File> saved_file)
        {
            saved_file->stream.seek(0);
            saved_files.push_back(saved_file);
        });

    const auto name_list = registry.get_decoder_names();
    flow::ParallelUnpackerContext context(
        dummy_logger,
        file_saver,
        registry,
        enable_nested_decoding,
        {},
        std::set<std::string>(name_list.begin(), name_list.end()));

    flow::ParallelUnpacker unpacker(context);
    unpacker.add_input_file(
        input_file.path,
        [&]()
        {
            return std::make_shared<io::File>(input_file);
        });
    unpacker.run(1);
    return saved_files;
}
