#ifndef FORMATS_GLIB_PGX_CONVERTER_H
#define FORMATS_GLIB_PGX_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Glib
    {
        class PgxConverter final : public Converter
        {
        protected:
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
