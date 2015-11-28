#pragma once

#include "fmt/shiina_rio/warc/plugin.h"

namespace au {
namespace fmt {
namespace shiina_rio {
namespace warc {

    void decrypt_essential(
        const Plugin &plugin, const int warc_version, bstr &data);

    FlagCryptFunc decrypt_with_flags1(const u32 key);
    FlagCryptFunc decrypt_with_flags2();
    FlagCryptFunc decrypt_with_flags3();

    void decrypt_with_crc(const Plugin &plugin, bstr &data);

    void decrypt_table_data(
        const Plugin &plugin,
        const int warc_version,
        const size_t table_offset,
        bstr &table_data);

} } } }
