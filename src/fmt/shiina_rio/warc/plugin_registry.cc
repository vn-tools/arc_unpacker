#include "fmt/shiina_rio/warc/plugin_registry.h"
#include "fmt/png/png_image_decoder.h"
#include "fmt/shiina_rio/warc/decrypt.h"
#include "io/file.h"
#include "io/program_path.h"
#include "util/plugin_mgr.h"

using namespace au;
using namespace au::fmt::shiina_rio::warc;

static bstr read_file(const std::string &name)
{
    const auto path = io::get_assets_dir_path() / "shiina_rio" / name;
    io::File file(path, io::FileMode::Read);
    return file.stream.seek(0).read_to_eof();
}

static std::shared_ptr<res::Image> read_image(const std::string &name)
{
    io::File tmp_file("tmp.png", read_file(name));
    const fmt::png::PngImageDecoder png_decoder;
    return std::make_shared<res::Image>(png_decoder.decode(tmp_file));
}

struct PluginRegistry::Priv final
{
    using PluginBuilder = std::function<std::shared_ptr<Plugin>()>;
    util::PluginManager<PluginBuilder> plugin_mgr;
    std::shared_ptr<Plugin> plugin;
};

PluginRegistry::PluginRegistry() : p(new Priv)
{
    p->plugin_mgr.add(
        "237",
        "Generic ShiinaRio v2.37", []()
        {
            auto p = std::make_shared<Plugin>();
            p->version = 2370;
            p->entry_name_size = 0x10;
            p->region_image = read_image("region.png");
            p->logo_data = read_file("logo1.png");
            p->initial_crypt_base_keys
                = {0xF182C682, 0xE882AA82, 0x718E5896, 0x8183CC82, 0xDAC98283};
            p->crc_crypt = get_crc_crypt(read_file("table1.bin"));
            return p;
        });

    p->plugin_mgr.add(
        "shojo-mama",
        "Shojo Mama", []()
        {
            auto p = std::make_shared<Plugin>();
            p->version = 2490;
            p->entry_name_size = 0x20;
            p->region_image = read_image("region.png");
            p->logo_data = read_file("logo3.jpg");
            p->crc_crypt = get_crc_crypt(read_file("table4.bin"));
            p->initial_crypt_base_keys = {0x4B535453, 0xA15FA15F, 0, 0, 0};
            p->flag_pre_crypt
                = p->flag_post_crypt
                = warc::get_flag_crypt1(read_file("flag.png"), 0xECB2F5B2);
            return p;
        });

    p->plugin_mgr.add(
        "majime1",
        "Majime to Sasayakareru Ore wo Osananajimi no Risa ga Seiteki na Imi "
        "mo Komete Kanraku Shite Iku Hanashi (sic)",
        []()
        {
            auto p = std::make_shared<Plugin>();
            p->version = 2490;
            p->entry_name_size = 0x20;
            p->region_image = read_image("region.png");
            p->logo_data = read_file("logo4.jpg");
            p->initial_crypt_base_keys
                = {0xF1AD65AB, 0x55B7E1AD, 0x62B875B8, 0, 0};
            p->flag_pre_crypt = warc::get_flag_crypt2();
            p->crc_crypt = get_crc_crypt(read_file("table4.bin"));
            return p;
        });

    p->plugin_mgr.add(
        "sorcery-jokers",
        "Sorcery Jokers", []()
        {
            auto p = std::make_shared<Plugin>();
            p->version = 2500;
            p->entry_name_size = 0x20;
            p->region_image = read_image("region.png");
            p->logo_data = read_file("logo5.jpg");
            p->initial_crypt_base_keys = {0x6C877787, 0x00007787, 0, 0, 0};
            p->flag_post_crypt = warc::get_flag_crypt3();
            p->crc_crypt = get_crc_crypt(read_file("table4.bin"));
            return p;
        });

    p->plugin_mgr.add(
        "gh-nurse",
        "Gohoushi Nurse", []()
        {
            auto p = std::make_shared<Plugin>();
            p->version = 2500;
            p->entry_name_size = 0x20;
            p->region_image = read_image("region.png");
            p->logo_data = read_file("logo6.jpg");
            p->initial_crypt_base_keys
                = {0xEFED26E8, 0x8CF5A1EE, 0x13E9D4EC, 0, 0};
            p->flag_pre_crypt
                = p->flag_post_crypt
                = warc::get_flag_crypt1(read_file("flag.png"), 0x90CC9DC2);
            p->crc_crypt = get_crc_crypt(read_file("table4.bin"));
            return p;
        });
}

PluginRegistry::~PluginRegistry()
{
}

void PluginRegistry::register_cli_options(ArgParser &arg_parser) const
{
    p->plugin_mgr.register_cli_options(
        arg_parser, "Selects NPA decryption routine.");
}

void PluginRegistry::parse_cli_options(const ArgParser &arg_parser)
{
    p->plugin_mgr.parse_cli_options(arg_parser);
}

std::shared_ptr<Plugin> PluginRegistry::get_plugin() const
{
    return p->plugin_mgr.get()();
}
