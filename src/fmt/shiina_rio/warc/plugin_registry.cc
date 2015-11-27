#include "fmt/shiina_rio/warc/plugin_registry.h"
#include "fmt/png/png_image_decoder.h"
#include "fmt/shiina_rio/warc/decrypt.h"
#include "io/file.h"
#include "util/encoding.h"
#include "util/plugin_mgr.hh"
#include "util/program_path.h"

using namespace au;
using namespace au::fmt::shiina_rio::warc;

static bstr read_file(const std::string &name)
{
    const io::path path = util::get_extra_dir_path() / "shiina_rio" / name;
    io::File file(path, io::FileMode::Read);
    return file.stream.read_to_eof();
}

static std::shared_ptr<pix::Image> read_image(const std::string &name)
{
    fmt::png::PngImageDecoder png_decoder;
    const auto content = read_file(name);
    io::File tmp_file("tmp.png", content);
    return std::make_shared<pix::Image>(png_decoder.decode(tmp_file));
}

static std::shared_ptr<Plugin> create_plugin(
    int version, const std::array<u32, 5> &initial_crypt_base_keys)
{
    auto plugin = std::make_shared<Plugin>();
    plugin->version = version;
    plugin->entry_name_size = version <= 2390 ? 0x10 : 0x20;
    plugin->initial_crypt_base_keys = initial_crypt_base_keys;
    plugin->essential_crypt_key = util::utf8_to_sjis(
        "Crypt Type 20011002 - Copyright(C) 2000 Y.Yamada/STUDStream よしくん"_b);
    plugin->region_image = read_image("region.png");

    plugin->crc_crypt.table
        = version < 2410 ? read_file("table1.bin")
        : version < 2490 ? read_file("table2.bin")
        : version < 2450 ? read_file("table3.bin")
        : read_file("table4.bin");
    plugin->logo_data
        = version < 2390 ? read_file("logo1.png")
        : version < 2490 ? read_file("logo2.jpg")
        : read_file("logo3.jpg");
    plugin->flag_crypt.table = read_file("flag.png");
    return plugin;
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
            auto plugin = create_plugin(
                2370,
                {0xF182C682, 0xE882AA82, 0x718E5896, 0x8183CC82, 0xDAC98283});
            return plugin;
        });

    p->plugin_mgr.add(
        "shojo-mama",
        "Shojo Mama", []()
        {
            auto plugin = create_plugin(
                2490, {0x4B535453, 0xA15FA15F, 0, 0, 0});
            plugin->flag_crypt.pre = warc::decrypt_with_flags1;
            plugin->flag_crypt.post = warc::decrypt_with_flags1;
            return plugin;
        });

    p->plugin_mgr.add(
        "majime1",
        "Majime to Sasayakareru Ore wo Osananajimi no Risa ga Seiteki na Imi "
        "mo Komete Kanraku Shite Iku Hanashi (sic)",
        []()
        {
            auto plugin = create_plugin(
                2490, {0xF1AD65AB, 0x55B7E1AD, 0x62B875B8, 0, 0});
            plugin->logo_data = read_file("logo4.jpg").substr(0, 0xBFAE);
            plugin->flag_crypt.pre = nullptr;
            plugin->flag_crypt.pre = warc::decrypt_with_flags2;
            return plugin;
        });

    p->plugin_mgr.add(
        "sorcery-jokers",
        "Sorcery Jokers", []()
        {
            auto plugin = create_plugin(
                2500, {0x6C877787, 0x00007787, 0, 0, 0});
            plugin->logo_data = read_file("logo5.jpg");
            plugin->flag_crypt.pre = nullptr;
            plugin->flag_crypt.post = warc::decrypt_with_flags3;
            return plugin;
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
