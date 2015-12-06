#pragma once

#include <array>
#include <memory>
#include "types.h"

namespace au {
namespace util {
namespace crypt {

    struct RsaKey final
    {
        std::array<u8, 64> modulus;
        unsigned int exponent;
    };

    class Rsa final
    {
    public:
        Rsa(const RsaKey &key);
        ~Rsa();
        bstr decrypt(const bstr &input) const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
