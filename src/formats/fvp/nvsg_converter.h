#ifndef AU_FMT_FVP_NVSG_CONVERTER_H
#define AU_FMT_FVP_NVSG_CONVERTER_H
#include "formats/converter.h"

namespace au {
namespace fmt {
namespace fvp {

    class NvsgConverter final : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }

#endif
