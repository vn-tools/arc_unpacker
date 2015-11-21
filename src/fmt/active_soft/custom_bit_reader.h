#pragma once

#include <memory>
#include "types.h"

namespace au {
namespace fmt {
namespace active_soft {

    class CustomBitReader final
    {
    public:
        CustomBitReader(const bstr &input);
        ~CustomBitReader();
        u32 get(const size_t bits);
        u32 get_variable_integer();

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
