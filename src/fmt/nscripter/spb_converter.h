#ifndef AU_FMT_NSCRIPTER_SPB_CONVERTER_H
#define AU_FMT_NSCRIPTER_SPB_CONVERTER_H
#include "fmt/converter.h"

namespace au {
namespace fmt {
namespace nscripter {

    class SpbConverter final : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }

#endif
