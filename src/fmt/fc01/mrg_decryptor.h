#pragma once

#include <memory>
#include "io/io.h"

namespace au {
namespace fmt {
namespace fc01 {

    class MrgDecryptor final
    {
    public:
        MrgDecryptor(const bstr &input);
        ~MrgDecryptor();
        bstr decrypt(u8 initial_key);
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
