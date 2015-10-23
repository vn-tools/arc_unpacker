#pragma once

#include "fmt/shiina_rio/warc/plugin.h"

namespace au {
namespace fmt {
namespace shiina_rio {
namespace warc {

    void decrypt_essential(
        const Plugin &plugin, const int warc_version, bstr &data);
    void decrypt_with_flags1(const Plugin &plugin, bstr &data, const u32 flags);
    void decrypt_with_flags2(const Plugin &plugin, bstr &data, const u32 flags);
    void decrypt_with_flags3(const Plugin &plugin, bstr &data, const u32 flags);
    void decrypt_with_crc(const Plugin &plugin, bstr &data);

    void decrypt_table_data(
        const Plugin &plugin,
        const int warc_version,
        const size_t table_offset,
        bstr &table_data);

} } } }
