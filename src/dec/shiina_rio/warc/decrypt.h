#pragma once

#include "dec/shiina_rio/warc/plugin.h"

namespace au {
namespace dec {
namespace shiina_rio {
namespace warc {

    void decrypt_essential(
        const Plugin &plugin, const int warc_version, bstr &data);

    void crc_crypt(bstr &data, const bstr &table);

    void decrypt_table_data(
        const Plugin &plugin,
        const int warc_version,
        const size_t table_offset,
        bstr &table_data);

} } } }
