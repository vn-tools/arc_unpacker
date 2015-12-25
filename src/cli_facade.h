#pragma once

#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include "logger.h"

namespace au {

    class CliFacade final
    {
    public:
        CliFacade(
            Logger &logger,
            const std::vector<std::string> &arguments);

        ~CliFacade();

        int run() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
