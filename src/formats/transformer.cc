#include <boost/filesystem/path.hpp>
#include <stdexcept>
#include "formats/transformer.h"

std::string FileNameDecorator::decorate(
    const FileNamingStrategy &strategy,
    const std::string &parent_file_name,
    const std::string &current_file_name)
{
    switch (strategy)
    {
        case FileNamingStrategy::Root:
            return current_file_name;

        case FileNamingStrategy::Child:
        case FileNamingStrategy::Sibling:
        {
            if (parent_file_name == "")
                return current_file_name;
            auto path = boost::filesystem::path(parent_file_name);
            if (strategy == FileNamingStrategy::Sibling)
                path = path.parent_path();
            path /= current_file_name;
            return path.string();
        }

        default:
            throw std::runtime_error("Invalid file naming strategy");
    }
}

void Transformer::add_cli_help(ArgParser &arg_parser) const
{
    for (auto &transformer : transformers)
    {
        if (transformer != this)
            transformer->add_cli_help(arg_parser);
    }
}

void Transformer::parse_cli_options(const ArgParser &arg_parser)
{
    for (auto &transformer : transformers)
    {
        if (transformer != this)
            transformer->parse_cli_options(arg_parser);
    }
}

void Transformer::add_transformer(Transformer *transformer)
{
    transformers.push_back(transformer);
}

bool Transformer::try_unpack(File &file, FileSaver &file_saver) const
{
    try
    {
        unpack(file, file_saver);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void Transformer::unpack(File &file, FileSaver &file_saver) const
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

Transformer::~Transformer()
{
}
