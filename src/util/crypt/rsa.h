#ifndef AU_UTIL_CRYPT_RSA_H
#define AU_UTIL_CRYPT_RSA_H

#include <memory>
#include <string>
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
        std::string decrypt(const std::string &input) const;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif

