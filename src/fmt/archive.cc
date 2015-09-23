#include "fmt/archive.h"
#include "err.h"

using namespace au;
using namespace au::fmt;

namespace
{
    struct DepthKeeper final
    {
        DepthKeeper();
        ~DepthKeeper();
    };
}

static const int max_depth = 10;
static int depth = 0;

DepthKeeper::DepthKeeper()
{
    depth++;
}

DepthKeeper::~DepthKeeper()
{
    depth--;
}

static bool pass_through_transformers(
    FileSaverCallback &recognition_proxy,
    std::shared_ptr<File> original_file,
    std::vector<Transformer*> transformers)
{
    for (auto &transformer : transformers)
    {
        FileSaverCallback transformer_proxy(
            [original_file, &recognition_proxy, &transformer]
            (std::shared_ptr<File> converted_file)
        {
            converted_file->name =
                FileNameDecorator::decorate(
                    transformer->get_file_naming_strategy(),
                    original_file->name,
                    converted_file->name);
            recognition_proxy.save(converted_file);
        });

        try
        {
            transformer->unpack(*original_file, transformer_proxy, true);
            return true;
        }
        catch (...)
        {
        }
    }

    return false;
}

void Archive::unpack(File &file, FileSaver &file_saver, bool recurse) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();

    // every file should be passed through registered transformers
    FileSaverCallback recognition_proxy;
    recognition_proxy.set_callback([&](std::shared_ptr<File> original_file)
    {
        DepthKeeper keeper;

        bool save_normally;
        if (depth > max_depth || !recurse)
        {
            save_normally = true;
        }
        else
        {
            save_normally = !pass_through_transformers(
                recognition_proxy, original_file, transformers);
        }

        if (save_normally)
            file_saver.save(original_file);
    });

    file.io.seek(0);
    return unpack_internal(file, recognition_proxy);
}

void Archive::register_cli_options(ArgParser &arg_parser) const
{
    for (auto &transformer : transformers)
    {
        if (transformer != this)
            transformer->register_cli_options(arg_parser);
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
