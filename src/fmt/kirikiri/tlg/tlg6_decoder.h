#pragma once

#include "io/file.h"
#include "pix/image.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace tlg {

    class Tlg6Decoder final
    {
    public:
        pix::Image decode(io::File &file);
    };

} } } }
