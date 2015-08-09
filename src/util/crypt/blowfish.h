#ifndef AU_UTIL_CRYPT_BLOWFISH_H
#define AU_UTIL_CRYPT_BLOWFISH_H

#include <memory>
#include "bstr.h"

namespace au {
namespace util {
namespace crypt {

    class Blowfish
    {
    public:
        Blowfish(const bstr &key);
        ~Blowfish();
        static size_t block_size();
        bstr decrypt(const bstr &input) const;
        bstr encrypt(const bstr &input) const;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
