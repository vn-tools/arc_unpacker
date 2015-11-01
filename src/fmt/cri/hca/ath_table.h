#pragma once

#include <array>
#include <memory>
#include "types.h"

namespace au {
namespace fmt {
namespace cri {
namespace hca {

    class AthTable final
    {
        public:
            AthTable(const u16 type, const u32 key);
            ~AthTable();
            const std::array<u8, 0x80> &get() const;

        private:
            struct Priv;
            std::unique_ptr<Priv> p;
    };

} } } }
