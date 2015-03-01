#ifndef FORMATS_QLIE_ABMP7_H
#define FORMATS_QLIE_ABMP7_H
#include <memory>
#include <string>
#include <vector>
#include "file.h"

namespace Formats
{
    namespace QLiE
    {
        void abmp7_unpack(
            std::vector<std::unique_ptr<File>> &files, File &b_file);
    }
}

#endif
