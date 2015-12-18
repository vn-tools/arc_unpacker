#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>

namespace au {

    class ArcUnpacker final
    {
    public:
        ArcUnpacker(const std::vector<std::string> &arguments);
        ~ArcUnpacker();
        int run() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
