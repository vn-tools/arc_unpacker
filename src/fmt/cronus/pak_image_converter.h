#pragma once

#include "fmt/converter.h"

namespace au {
namespace fmt {
namespace cronus {

    class PakImageConverter final : public Converter
    {
    public:
        PakImageConverter();
        ~PakImageConverter();
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
