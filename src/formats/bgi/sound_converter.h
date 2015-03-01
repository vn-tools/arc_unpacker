#ifndef FORMATS_BGI_SOUND_CONVERTER_H
#define FORMATS_BGI_SOUND_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Bgi
    {
        class SoundConverter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
