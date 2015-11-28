#pragma once

#include "fmt/shiina_rio/warc/plugin.h"

namespace au {
namespace fmt {
namespace shiina_rio {
namespace warc {

    void decrypt_essential(
        const Plugin &plugin, const int warc_version, bstr &data);

    CrcCryptFunc get_crc_crypt(const bstr &table);
    FlagCryptFunc get_flag_crypt1(const bstr &table, const u32 key);
    FlagCryptFunc get_flag_crypt2();
    FlagCryptFunc get_flag_crypt3();

    void decrypt_table_data(
        const Plugin &plugin,
        const int warc_version,
        const size_t table_offset,
        bstr &table_data);

} } } }
