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

#include "dec/base_archive_decoder.h"
#include <algorithm>
#include <cmath>
#include "algo/format.h"
#include "dec/idecoder_visitor.h"
#include "err.h"

using namespace au;
using namespace au::dec;

algo::NamingStrategy BaseArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Child;
}

BaseArchiveDecoder::BaseArchiveDecoder() : numeric_file_names(false)
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser
                .register_flag({"--numeric-file-names"})
                ->set_description(
                    "Replaces file names with extensionless sequential "
                    "numbers. Useful for recovering archives with broken file "
                    "names and for scripting.");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_flag("--numeric-file-names"))
                numeric_file_names = true;
        });
}

void BaseArchiveDecoder::accept(IDecoderVisitor &visitor) const
{
    visitor.visit(*this);
}

std::unique_ptr<ArchiveMeta> BaseArchiveDecoder::read_meta(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = read_meta_impl(logger, input_file);

    const auto width = meta->entries.size() > 1
        ? std::max<int>(1, 1 + std::log10(meta->entries.size()))
        : 0;

    if (numeric_file_names)
    {
        int number = 0;
        for (const auto &entry : meta->entries)
            entry->path = algo::format("%0*d", width, number++);
    }
    else
    {
        std::string prefix;
        if (naming_strategy() == algo::NamingStrategy::Sibling)
            prefix = input_file.path.stem();
        else if (naming_strategy() == algo::NamingStrategy::FlatSibling)
            prefix = input_file.path.stem();
        else if (naming_strategy() == algo::NamingStrategy::Root)
            prefix = (input_file.path.parent() / input_file.path.stem()).str();

        if (prefix.empty())
            prefix = "unk";

        int number = 0;
        for (const auto &entry : meta->entries)
        {
            if (!entry->path.str().empty())
                continue;

            entry->path = meta->entries.size() > 1
                ? algo::format("%s_%0*d", prefix.c_str(), width, number++)
                : prefix;

            entry->path.change_extension("dat");
        }
    }

    return meta;
}

std::unique_ptr<io::File> BaseArchiveDecoder::read_file(
    const Logger &logger,
    io::File &input_file,
    const ArchiveMeta &e,
    const ArchiveEntry &m) const
{
    // wrapper reserved for future usage
    return read_file_impl(logger, input_file, e, m);
}
