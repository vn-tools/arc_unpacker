#ifndef AU_ARC_UNPACKER_H
#define AU_ARC_UNPACKER_H
#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "formats/transformer.h"
#include "formats/transformer_factory.h"

namespace au {

    class ArcUnpacker
    {
    public:
        ArcUnpacker(ArgParser &, const std::string &version);
        ~ArcUnpacker();
        bool run();
        void print_help(const std::string &);
    private:
        std::unique_ptr<fmt::Transformer> guess_transformer(File &) const;
        bool guess_transformer_and_unpack(File &, const std::string &) const;
        void unpack(fmt::Transformer &, File &, const std::string &) const;
        void unpack(fmt::Transformer &, File &, const std::string &,
            FileSaver &) const;
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}

#endif
