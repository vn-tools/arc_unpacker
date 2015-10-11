#pragma once

#include "fmt/glib/glib2/plugin.h"

namespace au {
namespace fmt {
namespace glib {
namespace glib2  {

    class MusumePlugin final : public Plugin
    {
    public:
        std::unique_ptr<Decoder> create_header_decoder() const override;
        std::unique_ptr<Decoder> create_decoder(
            const std::array<u32, 4> &keys) const override;
    };

} } } }
