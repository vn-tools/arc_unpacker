// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/shiina_rio/warc_archive_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "dec/shiina_rio/warc/decrypt.h"
#include "io/file.h"
#include "io/memory_byte_stream.h"
#include "io/program_path.h"

using namespace au;
using namespace au::dec::shiina_rio;

static bstr read_etc_file(const std::string &name)
{
    const auto path = io::get_assets_dir_path() / "shiina_rio" / name;
    io::File file(path, io::FileMode::Read);
    return file.stream.seek(0).read_to_eof();
}

static std::shared_ptr<res::Image> read_etc_image(const std::string &name)
{
    Logger dummy_logger;
    dummy_logger.mute();
    io::File tmp_file("tmp.png", read_etc_file(name));
    const auto png_decoder = dec::png::PngImageDecoder();
    return std::make_shared<res::Image>(
        png_decoder.decode(dummy_logger, tmp_file));
}

namespace
{
    struct MakiFesExtraCrypt final : public warc::BaseExtraCrypt
    {
        size_t min_size() const override
        {
            return 0x400;
        }

        void pre_decrypt(bstr &data) const override
        {
            u32 key = 0x12BDB19B;
            const auto decode_table = read_etc_file("extra_makifes.png");
            for (const auto i : algo::range(0x100))
            {
                key = 0x343FD * key + 0x269EC3;
                data.at(i) ^= decode_table.at(
                    ((key >> 16) % 0x8000) % decode_table.size());
            }
        }

        void post_decrypt(bstr &data) const override
        {
            data.get<u32>()[0x80] ^= 0x12BDB19B;
        }
    };

    struct BitchNeechanExtraCrypt final : public warc::BaseExtraCrypt
    {
        size_t min_size() const override
        {
            return 0x400;
        }

        void pre_decrypt(bstr &data) const override
        {
            data.get<u32>()[0x80] ^= calc_key(data.get<u8>(), 255);
        }

        void post_decrypt(bstr &data) const override
        {
        }

        unsigned int calc_key(u8 *data, unsigned int size) const
        {
            unsigned int sum1 = 1;
            unsigned int sum2 = 0;
            for (const auto i : algo::range(size))
            {
                sum1 += data[i];
                sum2 += sum1;
            }
            sum1 %= 0xFFF1u;
            sum2 %= 0xFFF1u;
            return sum1 | (sum2 << 16);
        }
    };

    struct TableExtraCrypt final : public warc::BaseExtraCrypt
    {
        TableExtraCrypt(const bstr &table, const u32 seed)
            : table(table), seed(seed)
        {
        }

        size_t min_size() const override
        {
            return 0x400;
        }

        void pre_decrypt(bstr &data) const override
        {
            u32 k[4] = {seed + 1, seed + 4, seed + 2, seed + 3};
            for (const auto i : algo::range(0xFF))
            {
                const u32 j = k[3]
                    ^ (k[3] << 11)
                    ^ k[0]
                    ^ ((k[3] ^ (k[3] << 11) ^ (k[0] >> 11)) >> 8);
                k[3] = k[2];
                k[2] = k[1];
                k[1] = k[0];
                k[0] = j;
                const auto idx = j % table.size();
                data[i] ^= table[idx];
            }
        }

        void post_decrypt(bstr &data) const override
        {
            data.get<u32>()[0x80] ^= seed;
        }

    private:
        bstr table;
        u32 seed;
    };

    struct RevolveExtraCrypt final : public warc::BaseExtraCrypt
    {
        size_t min_size() const override
        {
            return 0x200;
        }

        void pre_decrypt(bstr &data) const override
        {
            u16 acc = 0;
            u16 revolve = 0;
            u16 work;
            for (const auto i : algo::range(0x100))
            {
                work = data[i];
                acc += work >> 1;
                data[i] = (work >> 1) | revolve;
                revolve = (work & 1) << 7;
            }
            data[0] |= (work & 1) << 7;
            data.get<u32>()[0x41] ^= acc;
        }

        void post_decrypt(bstr &data) const override
        {
        }
    };

    struct SorceryJokersExtraCrypt final : public warc::BaseExtraCrypt
    {
        size_t min_size() const override
        {
            return 0x400;
        }

        void pre_decrypt(bstr &data) const override
        {
        }

        void post_decrypt(bstr &data) const override
        {
            if (data.get<u32>()[0] == 0x718E958D)
                custom_decrypt(data);
            data.get<u32>()[0x80] ^= data.size();
        }

    private:
        void custom_decrypt(bstr &data) const
        {
            io::MemoryByteStream data_stream(data);
            data_stream.skip(8);

            const auto size = data_stream.read_le<u32>();
            int base_table[256];
            int diff_table_buf[257];
            int *diff_table = &diff_table_buf[1]; // permit access via x[-1]

            diff_table[-1] = 0;
            for (const auto i : algo::range(256))
            {
                base_table[i] = data_stream.read<u8>();
                diff_table[i] = base_table[i] + diff_table[i - 1];
            }

            bstr buffer(diff_table[0xFF]);
            for (const auto i : algo::range(256))
            {
                const auto offset1 = diff_table[i - 1];
                const auto offset2 = diff_table[i];
                for (const auto j : algo::range(offset2 - offset1))
                    buffer[offset1 + j] = i;
            }

            u32 unk0 = 0;
            u32 unk1 = 0xFFFFFFFF;
            u32 unk2 = data_stream.read_be<u32>();

            for (const auto i : algo::range(size))
            {
                const u32 scale = unk1 / buffer.size();
                const size_t idx = buffer.at((unk2 - unk0) / scale);
                data[i] = idx;
                unk0 += diff_table[idx - 1] * scale;
                unk1 = base_table[idx] * scale;
                while (!((unk0 ^ (unk1 + unk0)) & 0xFF000000))
                {
                    unk0 <<= 8;
                    unk1 <<= 8;
                    unk2 = (unk2 << 8) | data_stream.read<u8>();
                }
                while (unk1 < 0x10000)
                {
                    unk0 <<= 8;
                    unk1 = 0x1000000 - (unk0 & 0xFFFFFF);
                    unk2 = (unk2 << 8) | data_stream.read<u8>();
                }
            }
        }
    };
}

WarcArchiveDecoder::WarcArchiveDecoder()
{
    plugin_manager.add(
        "237",
        "Generic ShiinaRio v2.37",
        []()
        {
            auto p = std::make_shared<warc::Plugin>();
            p->version = 2370;
            p->entry_name_size = 0x10;
            p->region_image = read_etc_image("region.png");
            p->logo_data = read_etc_file("logo_237.png");
            p->initial_crypt_base_keys
                = {0xF182C682, 0xE882AA82, 0x718E5896, 0x8183CC82, 0xDAC98283};
            p->crc_crypt_source = read_etc_file("table1.bin");
            return p;
        });

    plugin_manager.add(
        "shojo-mama",
        "Shojo Mama",
        []()
        {
            auto p = std::make_shared<warc::Plugin>();
            p->version = 2490;
            p->entry_name_size = 0x20;
            p->region_image = read_etc_image("region.png");
            p->logo_data = read_etc_file("logo_shojo_mama.jpg");
            p->initial_crypt_base_keys = {0x4B535453, 0xA15FA15F, 0, 0, 0};
            p->extra_crypt = std::make_unique<TableExtraCrypt>(
                read_etc_file("extra_table.png"), 0xECB2F5B2);
            return p;
        });

    plugin_manager.add(
        "majime1",
        "Majime to Sasayakareru Ore wo Osananajimi no Risa ga Seiteki na Imi "
        "mo Komete Kanraku Shite Iku Hanashi (sic)",
        []()
        {
            auto p = std::make_shared<warc::Plugin>();
            p->version = 2490;
            p->entry_name_size = 0x20;
            p->region_image = read_etc_image("region.png");
            p->logo_data = read_etc_file("logo_majime1.jpg");
            p->initial_crypt_base_keys
                = {0xF1AD65AB, 0x55B7E1AD, 0x62B875B8, 0, 0};
            p->extra_crypt = std::make_unique<RevolveExtraCrypt>();
            return p;
        });

    plugin_manager.add(
        "sorcery-jokers",
        "Sorcery Jokers",
        []()
        {
            auto p = std::make_shared<warc::Plugin>();
            p->version = 2500;
            p->entry_name_size = 0x20;
            p->region_image = read_etc_image("region.png");
            p->logo_data = read_etc_file("logo_sorcery_jokers.jpg");
            p->initial_crypt_base_keys = {0x6C877787, 0x00007787, 0, 0, 0};
            p->extra_crypt = std::make_unique<SorceryJokersExtraCrypt>();
            p->crc_crypt_source = read_etc_file("table4.bin");
            return p;
        });

    plugin_manager.add(
        "gh-nurse",
        "Gohoushi Nurse",
        []()
        {
            auto p = std::make_shared<warc::Plugin>();
            p->version = 2500;
            p->entry_name_size = 0x20;
            p->region_image = read_etc_image("region.png");
            p->logo_data = read_etc_file("logo_gohoushi_nurse.jpg");
            p->initial_crypt_base_keys
                = {0xEFED26E8, 0x8CF5A1EE, 0x13E9D4EC, 0, 0};
            p->extra_crypt = std::make_unique<TableExtraCrypt>(
                read_etc_file("extra_table.png"), 0x90CC9DC2);
            return p;
        });

    plugin_manager.add(
        "gensou",
        "Gensou no Idea ~Oratorio Phantasm Historia~"
        "Gensou no Idea",
        []()
        {
            auto p = std::make_shared<warc::Plugin>();
            p->version = 2490;
            p->entry_name_size = 0x20;
            p->region_image = read_etc_image("region.png");
            p->logo_data = read_etc_file("logo_gensou_no_idea.jpg");
            p->initial_crypt_base_keys
                = {0x45BA9DA7, 0x68A8E7A9, 0x6AA84DA8, 0, 0};
            p->extra_crypt = std::make_unique<RevolveExtraCrypt>();
            return p;
        });

    plugin_manager.add(
        "maki-fes",
        "Maki Fes",
        []()
        {
            auto p = std::make_shared<warc::Plugin>();
            p->version = 2500;
            p->entry_name_size = 0x20;
            p->region_image = read_etc_image("region.png");
            p->logo_data = read_etc_file("logo_maki_fes.jpg");
            p->initial_crypt_base_keys = {0xF6DF81DF, 0x1BDE29DE, 0x5DE, 0, 0};
            p->extra_crypt = std::make_unique<MakiFesExtraCrypt>();
            return p;
        });

    plugin_manager.add(
        "bitch-neechan",
        "Bitch Nee-chan ga Seijun na Hazu ga Nai!",
        []()
        {
            auto p = std::make_shared<warc::Plugin>();
            p->version = 2500;
            p->entry_name_size = 0x20;
            p->region_image = read_etc_image("region.png");
            p->logo_data = read_etc_file("logo_bitch_neechan.jpg");
            p->initial_crypt_base_keys
                = {0x0FEE1FEE, 0x02E30DEE, 0x8CEFD2EF, 0xC7EF9CEF, 0xEEE2D9FD};
            p->extra_crypt = std::make_unique<BitchNeechanExtraCrypt>();
            p->crc_crypt_source = read_etc_file("table4.bin");
            return p;
        });

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects WARC decryption routine."));
}
