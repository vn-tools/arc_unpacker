#pragma once

#include "file.h"
#include "pix/grid.h"

namespace au {
namespace fmt {
namespace kirikiri {
namespace tlg {

    class Tlg6Decoder final
    {
    public:
        pix::Grid decode(File &file);
    };

} } } }
