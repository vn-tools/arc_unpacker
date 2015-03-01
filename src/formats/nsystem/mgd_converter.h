#ifndef FORMATS_NSYSTEM_MGD_CONVERTER_H
#define FORMATS_NSYSTEM_MGD_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace NSystem
    {
        class MgdConverter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
