#ifndef ARC_UNPACKER_H
#define ARC_UNPACKER_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "factory/transformer_factory.h"
#include "formats/transformer.h"

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
    void print_help(const std::string &);
    bool guess_transformer_and_unpack(File &, const std::string &);
    void unpack(Transformer &, File &, const std::string &);
    void unpack(Transformer &, File &, const std::string &, FileSaver &);
    bool run();
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
