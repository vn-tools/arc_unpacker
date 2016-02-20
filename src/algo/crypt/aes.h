#pragma once

#include "types.h"

namespace au {
namespace algo {
namespace crypt {

    bstr aes256_decrypt_cbc(const bstr &input, const bstr &iv, const bstr &key);
    bstr aes256_encrypt_cbc(const bstr &input, const bstr &iv, const bstr &key);

} } }
