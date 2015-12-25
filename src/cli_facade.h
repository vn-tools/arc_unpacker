#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>

namespace au {

    class CliFacade final
    {
    public:
        CliFacade(const std::vector<std::string> &arguments);
        ~CliFacade();
        int run() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
