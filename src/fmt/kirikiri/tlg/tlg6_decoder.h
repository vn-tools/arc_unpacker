#pragma once

#include "file.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace tlg {

    class Tlg6Decoder final
    {
    public:
        std::unique_ptr<File> decode(File &file);
    };

} } } }
