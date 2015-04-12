#ifndef ARC_UNPACKER_H
#define ARC_UNPACKER_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "formats/transformer.h"
#include "formats/transformer_factory.h"

typedef struct
{
    std::string input_path;
    std::string base_name;
} PathInfo;

class ArcUnpacker
{
public:
    ArcUnpacker(ArgParser &);
    ~ArcUnpacker();
    bool run();
    void print_help(const std::string &);
private:
    std::unique_ptr<Transformer> guess_transformer(File &) const;
    bool guess_transformer_and_unpack(File &, const std::string &) const;
    void unpack(Transformer &, File &, const std::string &) const;
    void unpack(Transformer &, File &, const std::string &, FileSaver &) const;
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
