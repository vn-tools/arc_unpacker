#ifndef FORMATS_GLIB_PGX_CONVERTER_H
#define FORMATS_GLIB_PGX_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Glib
    {
        class PgxConverter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
