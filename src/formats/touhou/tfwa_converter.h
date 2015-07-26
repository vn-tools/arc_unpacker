#ifndef AU_FMT_TOUHOU_TFWA_CONVERTER_H
#define AU_FMT_TOUHOU_TFWA_CONVERTER_H
#include "formats/converter.h"
#include "formats/touhou/palette.h"

namespace au {
namespace fmt {
namespace touhou {

    class TfwaConverter : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }

#endif
