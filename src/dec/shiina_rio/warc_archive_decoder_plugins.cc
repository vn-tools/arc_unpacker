#include "dec/shiina_rio/warc_archive_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "dec/shiina_rio/warc/decrypt.h"
#include "io/file.h"
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
            p->logo_data = read_etc_file("logo1.png");
            p->initial_crypt_base_keys
                = {0xF182C682, 0xE882AA82, 0x718E5896, 0x8183CC82, 0xDAC98283};
            p->crc_crypt = warc::get_crc_crypt(read_etc_file("table1.bin"));
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
            p->logo_data = read_etc_file("logo3.jpg");
            p->initial_crypt_base_keys = {0x4B535453, 0xA15FA15F, 0, 0, 0};
            p->flag_pre_crypt
                = p->flag_post_crypt
                = warc::get_flag_crypt1(read_etc_file("flag.png"), 0xECB2F5B2);
            p->crc_crypt = warc::get_crc_crypt(read_etc_file("table4.bin"));
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
            p->logo_data = read_etc_file("logo4.jpg");
            p->initial_crypt_base_keys
                = {0xF1AD65AB, 0x55B7E1AD, 0x62B875B8, 0, 0};
            p->flag_pre_crypt = warc::get_flag_crypt2();
            p->crc_crypt = warc::get_crc_crypt(read_etc_file("table4.bin"));
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
            p->logo_data = read_etc_file("logo5.jpg");
            p->initial_crypt_base_keys = {0x6C877787, 0x00007787, 0, 0, 0};
            p->flag_post_crypt = warc::get_flag_crypt3();
            p->crc_crypt = warc::get_crc_crypt(read_etc_file("table4.bin"));
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
            p->logo_data = read_etc_file("logo6.jpg");
            p->initial_crypt_base_keys
                = {0xEFED26E8, 0x8CF5A1EE, 0x13E9D4EC, 0, 0};
            p->flag_pre_crypt
                = p->flag_post_crypt
                = warc::get_flag_crypt1(read_etc_file("flag.png"), 0x90CC9DC2);
            p->crc_crypt = warc::get_crc_crypt(read_etc_file("table4.bin"));
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
            p->logo_data = read_etc_file("logo7.jpg");
            p->initial_crypt_base_keys
                = {0x45BA9DA7, 0x68A8E7A9, 0x6AA84DA8, 0, 0};
            p->flag_pre_crypt = warc::get_flag_crypt2();
            p->crc_crypt = warc::get_crc_crypt(read_etc_file("table4.bin"));
            return p;
        });

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects WARC decryption routine."));
}
