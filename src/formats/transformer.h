#ifndef FORMATS_TRANSFORMER_H
#define FORMATS_TRANSFORMER_H
#include <memory>
#include <vector>
#include "arg_parser.h"
#include "file.h"
#include "file_saver.h"

enum class FileNamingStrategy : uint8_t
{
    Root = 1,
    Sibling = 2,
    Child = 3,
};

class FileNameDecorator
{
public:
    static std::string decorate(
        const FileNamingStrategy &strategy,
        const std::string &parent_file_name,
        const std::string &current_file_name);
};

class Transformer
{
public:
    virtual ~Transformer();

    bool try_unpack(File &, FileSaver &) const;
    void unpack(File &, FileSaver &) const;

    virtual void add_cli_help(ArgParser &) const;
    virtual void parse_cli_options(const ArgParser &);
    virtual FileNamingStrategy get_file_naming_strategy() const = 0;

protected:
    virtual void unpack_internal(File &, FileSaver &) const = 0;
    void add_transformer(Transformer *transformer);

private:
    std::vector<Transformer*> transformers;
};

#endif
