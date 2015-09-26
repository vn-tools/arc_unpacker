#pragma once

#include "fmt/file_decoder.h"

namespace au {
namespace fmt {
namespace cronus {

    class GrpImageDecoder final : public FileDecoder
    {
    public:
        GrpImageDecoder();
        ~GrpImageDecoder();
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
