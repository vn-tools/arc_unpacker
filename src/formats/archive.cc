#include "formats/archive.h"

void Archive::unpack(File &file, FileSaver &file_saver) const
{
    //every file should be passed through registered transformers
    FileSaverCallback file_saver_proxy([&](std::shared_ptr<File> saved_file)
    {
        for (auto &transformer : transformers)
        {
            FileSaverCallback nested_proxy;

            nested_proxy.set_callback(
                [saved_file, &file_saver_proxy, &transformer]
                (std::shared_ptr<File> nested_file)
            {
                nested_file->name =
                    FileNameDecorator::decorate(
                        transformer->get_file_naming_strategy(),
                        saved_file->name,
                        nested_file->name);
                file_saver_proxy.save(nested_file);
            });

            if (transformer->try_unpack(*saved_file, nested_proxy))
                return;
        }

        file_saver.save(saved_file);
    });

    file.io.seek(0);
    return unpack_internal(file, file_saver_proxy);
}

void Archive::add_cli_help(ArgParser &arg_parser) const
{
    for (auto &transformer : transformers)
    {
        if (transformer != this)
            transformer->add_cli_help(arg_parser);
    }
}

void Archive::parse_cli_options(const ArgParser &arg_parser)
{
    for (auto &transformer : transformers)
    {
        if (transformer != this)
            transformer->parse_cli_options(arg_parser);
    }
}

void Archive::add_transformer(Transformer *transformer)
{
    transformers.push_back(transformer);
}

FileNamingStrategy Archive::get_file_naming_strategy() const
{
    return FileNamingStrategy::Child;
}

Archive::~Archive()
{
}
