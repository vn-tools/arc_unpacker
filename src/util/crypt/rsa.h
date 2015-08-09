#ifndef AU_UTIL_CRYPT_RSA_H
#define AU_UTIL_CRYPT_RSA_H
#include <memory>
#include <array>
#include "types.h"

namespace au {
namespace util {
namespace crypt {

    struct RsaKey
    {
        std::array<u8, 64> modulus;
        unsigned int exponent;
    };

    class Rsa
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

#endif
