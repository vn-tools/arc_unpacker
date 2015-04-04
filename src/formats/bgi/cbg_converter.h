#ifndef FORMATS_BGI_CBG_CONVERTER_H
#define FORMATS_BGI_CBG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Bgi
    {
        class CbgConverter final : public Converter
        {
        protected:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
