#pragma once

#include "io/file.h"

namespace au {
namespace flow {

    class IFileSaver
    {
    public:
        virtual ~IFileSaver() {}
        virtual io::path save(std::shared_ptr<io::File> file) const = 0;
    };

} }
