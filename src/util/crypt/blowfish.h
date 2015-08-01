#ifndef AU_UTIL_CRYPT_BLOWFISH_H
#define AU_UTIL_CRYPT_BLOWFISH_H

#include <memory>
#include <string>

namespace au {
namespace util {
namespace crypt {

    class Blowfish
    {
    public:
        Blowfish(const std::string &key);
        ~Blowfish();
        static size_t block_size();
        std::string decrypt(const std::string &input) const;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }

#endif
