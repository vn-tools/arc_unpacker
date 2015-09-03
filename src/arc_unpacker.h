#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "fmt/transformer.h"

namespace au {

    class ArcUnpacker
    {
    public:
        ArcUnpacker(ArgParser &, const std::string &version);
        ~ArcUnpacker();
        bool run();
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
