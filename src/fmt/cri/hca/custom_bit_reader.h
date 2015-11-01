#pragma once

#include <memory>
#include "types.h"

namespace au {
namespace fmt {
namespace cri {
namespace hca {

    class CustomBitReader final
    {
        public:
            CustomBitReader(const bstr &data);
            ~CustomBitReader();
            int get(const size_t n);
            void skip(const int n);
        private:
            struct Priv;
            std::unique_ptr<Priv> p;
    };

} } } }
