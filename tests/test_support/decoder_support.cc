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

#include "test_support/decoder_support.h"
#include "test_support/catch.h"

using namespace au;

// This is to test whether ImageDecoder::decode, IDecoder::is_recognized etc.
// take care of stream position themselves rather than relying on the callers.
static void navigate_to_random_place(io::BaseByteStream &input_stream)
{
    if (input_stream.size())
        input_stream.seek(rand() % input_stream.size());
}

std::vector<std::shared_ptr<io::File>> tests::unpack(
    const dec::BaseArchiveDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    Logger dummy_logger;
    dummy_logger.mute();
    const auto meta = decoder.read_meta(dummy_logger, input_file);
    std::vector<std::shared_ptr<io::File>> files;
    for (const auto &entry : meta->entries)
    {
        files.push_back(decoder.read_file(
            dummy_logger, input_file, *meta, *entry));
    }
    return files;
}

std::unique_ptr<io::File> tests::decode(
    const dec::BaseFileDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    Logger dummy_logger;
    dummy_logger.mute();
    return decoder.decode(dummy_logger, input_file);
}

res::Image tests::decode(
    const dec::BaseImageDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    Logger dummy_logger;
    dummy_logger.mute();
    return decoder.decode(dummy_logger, input_file);
}

res::Audio tests::decode(
    const dec::BaseAudioDecoder &decoder, io::File &input_file)
{
    navigate_to_random_place(input_file.stream);
    REQUIRE(decoder.is_recognized(input_file));
    navigate_to_random_place(input_file.stream);
    Logger dummy_logger;
    dummy_logger.mute();
    return decoder.decode(dummy_logger, input_file);
}
