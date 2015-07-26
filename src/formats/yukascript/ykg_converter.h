#ifndef AU_FMT_YUKASCRIPT_YKG_CONVERTER_H
#define AU_FMT_YUKASCRIPT_YKG_CONVERTER_H
#include "formats/converter.h"

namespace au {
namespace fmt {
namespace yukascript {

    class YkgConverter final : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }

#endif
