#include "fmt/shiina_rio/warc/decrypt.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/endian.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::shiina_rio;
using namespace au::fmt::shiina_rio::warc;

static const unsigned int one_millisecond = 10000;
static const unsigned int days_per_year = 365;
static const unsigned int days_per_4_years = days_per_year * 4 + 1;
static const unsigned int days_per_100_years = days_per_4_years * 25 - 1;
static const unsigned int days_per_400_years = days_per_100_years * 4 + 1;
static const unsigned int year_day_acc[2][13] =
{
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 },
};

namespace
{
    class CustomLcg final
    {
    public:
        CustomLcg(const u32 seed);
        void reset(const u32 seed);

        template<typename T> T current()
        {
            return static_cast<T>(value);
        }

        template<typename T> T next()
        {
            step();
            return current<T>();
        }

    private:
        void step();
        u32 value;
    };
}

CustomLcg::CustomLcg(const u32 seed) : value(seed)
{
}

void CustomLcg::reset(const u32 seed)
{
    value = seed;
}

void CustomLcg::step()
{
    value = 0x5D588B65 * value + 1;
}

static bool is_leap_year(const unsigned int year)
{
    if ((year % 4) != 0)
        return false;
    if ((year % 100) == 0)
        return (year % 400) == 0;
    return true;
}

static size_t get_max_table_size(const Plugin &plugin, const int warc_version)
{
    const size_t max_index_entries = warc_version < 150 ? 8192 : 16384;
    return (plugin.entry_name_size + 0x18) * max_index_entries;
}

static inline u32 mix_colors(const u32 c1, const u32 c2, const u32 alpha)
{
    return c1 + (((c2 - c1) * alpha) >> 8);
}

static u32 calculate_crc32(const bstr &data)
{
    const u32 xor_value = 0x6DB88320;
    u32 table[0x100];
    for (const auto i : util::range(0x100))
    {
        u32 poly = i;
        for (const auto j : util::range(8))
        {
            const bool bit = poly & 1;
            poly = (poly >> 1) | (poly << 31);
            if (bit == 1)
                poly ^= xor_value;
        }
        table[i] = poly;
    }
    u32 checksum = 0xFFFFFFFF;
    for (const auto c : data)
        checksum = (checksum >> 8) ^ table[(c ^ (checksum & 0xFF)) & 0xFF];
    checksum ^= 0xFFFFFFFF;
    return checksum;
}

static pix::Grid transform_region_image(
    const pix::Grid &input_image, const u32 flags, const u32 base_color)
{
    pix::Grid output_image(input_image.width(), input_image.height());
    pix::Pixel base_pixel;
    base_pixel.b = base_color;
    base_pixel.g = base_color >> 8;
    base_pixel.r = base_color >> 16;
    const int src_alpha = (flags & 0x08000000) ? (flags & 0x1FF) : 0x100;
    const int dst_alpha = (flags & 0x10000000) ? ((flags >> 12) & 0x1FF) : 0;
    const bool flip_vertically = flags & 0x20000000;
    const bool flip_horizontally = flags & 0x40000000;
    for (const auto y : util::range(input_image.height()))
    for (const auto x : util::range(input_image.width()))
    {
        const auto src_y = flip_vertically ? input_image.height() - 1 - y : y;
        const auto src_x = flip_horizontally ? input_image.width() - 1 - x : x;
        auto &output_pixel = output_image.at(x, y);
        const auto &input_pixel = input_image.at(src_x, src_y);
        const auto alpha = mix_colors(0, input_pixel.a, src_alpha);
        for (const auto i : util::range(3))
        {
            output_pixel[i] = mix_colors(
                output_pixel[i],
                (mix_colors(input_pixel[i], base_pixel[i], dst_alpha)) & 0xFF,
                alpha);
        }
    }
    return output_image;
}

static bstr get_rgb_data(const pix::Grid &input_image)
{
    bstr output(input_image.width() * input_image.height() * 3);
    auto output_ptr = output.get<u8>();
    for (const auto c : input_image)
    {
        *output_ptr++ = c.b;
        *output_ptr++ = c.g;
        *output_ptr++ = c.r;
    }
    return output;
}

static double get_lcg_xor(const double a)
{
    if (a < 0)
    {
        return -get_lcg_xor(-a);
    }
    else if (a < 18.0)
    {
        double result = a;
        double unk1 = a;
        double unk2 = -(a * a);

        for (int j = 3; j < 1000; j += 2)
        {
            unk1 *= unk2 / static_cast<double>(j * (j - 1));
            result += unk1 / j;
            if (result == unk2)
                break;
        }
        return result;
    }
    else
    {
        int flags = 0;
        double unk0_l = 0;
        double unk1 = 0;
        double div = 1 / a;
        double unk1_h = 2.0;
        double unk0_h = 2.0;
        double unk1_l = 0;
        double unk0 = 0;
        int i = 0;

        do
        {
            unk0 += div;
            div *= ++i / a;
            if (unk0 < unk0_h)
                unk0_h = unk0;
            else
                flags |= 1;

            unk1 += div;
            div *= ++i / a;
            if (unk1 < unk1_h)
                unk1_h = unk1;
            else
                flags |= 2;

            unk0 -= div;
            div *= ++i / a;
            if (unk0 > unk0_l)
                unk0_l = unk0;
            else
                flags |= 4;

            unk1 -= div;
            div *= ++i / a;
            if (unk1 > unk1_l)
                unk1_l = unk1;
            else
                flags |= 8;
        }
        while (flags != 0xF);

        return (M_PI
            - (std::cos(a) * (unk0_l + unk0_h))
            - (std::sin(a) * (unk1_l + unk1_h))) / 2.0;
    }
}

static u64 get_essential_key_pos_addend1(const u64 key_pos)
{
    bstr input(4);
    *input.get<u32>() = key_pos;
    bstr s[4];
    for (const auto i : util::range(4))
    {
        s[i].resize(4);
        *s[i].get<float>() = 1.5 * input[i] + 0.1;
    }
    u32 d[4];
    d[0] = util::from_big_endian(*s[0].get<const u32>());
    d[1] = *s[1].get<const float>();
    d[2] = -*s[2].get<const s32>();
    d[3] = ~*s[3].get<const u32>();
    return (d[0] + d[1]) | (d[2] - d[3]);
}

static u64 get_essential_key_pos_addend2(const double a, CustomLcg &lcg)
{
    double ret;

    if (a > 1.0)
    {
        double unk0 = std::sqrt(a * 2.0 - 1.0);
        while (true)
        {
            double unk1 = 1.0 - lcg.next<u32>() / 4294967296.0;
            double unk2 = 2.0 * lcg.next<u32>() / 4294967296.0 - 1.0;
            if (unk1 * unk1 + unk2 * unk2 > 1.0)
                continue;

            unk2 /= unk1;
            ret = unk2 * unk0 + a - 1.0;
            if (ret <= 0.0)
                continue;

            unk1 = (a - 1.0) * std::log(ret / (a - 1.0)) - unk2 * unk0;
            if (unk1 < -50.0)
                continue;

            if ((lcg.next<u32>() / 4294967296.0)
                <= (std::exp(unk1) * (unk2 * unk2 + 1.0)))
            {
                break;
            }
        }
    }
    else
    {
        double unk0 = std::exp(1.0) / (a + std::exp(1.0));
        double unk1;
        do
        {
            unk1 = lcg.next<u32>() / 4294967296.0;
            double unk2 = lcg.next<u32>() / 4294967296.0;
            if (unk1 < unk0)
            {
                ret = std::pow(unk2, 1.0 / a);
                unk1 = std::exp(-ret);
            }
            else
            {
                ret = 1.0 - std::log(unk2);
                unk1 = std::pow(ret, a - 1.0);
            }
        }
        while (lcg.next<u32>() / 4294967296.0 >= unk1);
    }

    return ret * 256.0;
}

static std::array<u32, 5> get_initial_crypt_key_addends(
    const Plugin &plugin, const std::array<u32, 0x50> &buf)
{
    static const std::vector<u32> addend_consts
    {
        0x00000000,
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xA953FD4E
    };

    static const std::vector<std::function<u32(const std::array<u32, 5> &)>>
        addend_funcs
        {
            [](const std::array<u32, 5> &addends)
            {
                return addends[1] ^ addends[2] ^ addends[3];
            },
            [](const std::array<u32, 5> &addends)
            {
                return (addends[1] & addends[2]) | (addends[3] & ~addends[1]);
            },
            [](const std::array<u32, 5> &addends)
            {
                return addends[3] ^ (addends[1] | ~addends[2]);
            },
            [](const std::array<u32, 5> &addends)
            {
                return (addends[1] & addends[3]) | (addends[2] & ~addends[3]);
            },
            [](const std::array<u32, 5> &addends)
            {
                return addends[1] ^ (addends[2] | ~addends[3]);
            }
        };

    std::array<u32, 5> addends(plugin.initial_crypt_base_keys);
    for (const auto i : util::range(0x50))
    {
        const u32 tmp1
            = buf[i]
            + addends[4]
            + addend_funcs[i / 0x10](addends)
            + addend_consts[i / 0x10]
            + (addends[0] << 5 | addends[0] >> 27);
        const u32 tmp2 = (addends[1] >> 2) | (addends[1] << 30);
        addends[4] = addends[3];
        addends[3] = addends[2];
        addends[2] = tmp2;
        addends[1] = addends[0];
        addends[0] = tmp1;
    }
    return addends;
}

std::array<u32, 10> get_initial_crypt_keys(
    const Plugin &plugin, const bstr &data)
{
    io::BufferedIO data_io(data);
    std::array<u32, 0x50> buf;
    data_io.seek(44);
    for (const auto i : util::range(0x10))
        buf[i] = data_io.read_u32_be();
    for (const auto i : util::range(0x10, 0x50))
    {
        buf[i]
            = buf[i - 0x10]
            ^ buf[i - 0x0E]
            ^ buf[i - 0x08]
            ^ buf[i - 0x03];
        buf[i] = (buf[i] << 1) | (buf[i] >> 31);
    }

    const auto addends = get_initial_crypt_key_addends(plugin, buf);
    std::array<u32, 10> keys;
    for (const auto i : util::range(5))
        keys[i] = plugin.initial_crypt_base_keys[i] + addends[i];

    {
        const s32 key2_signed = keys[2];
        const s32 key3_signed = keys[3];
        const u64 key2_promoted = key2_signed;
        const u64 key3_promoted = key3_signed;
        keys[9] = (key2_promoted * key3_promoted) >> 8;
    }

    {
        u32 flags = util::from_big_endian(buf[0]) | 0x80000000;
        if (!(flags & 0x78000000))
            flags |= 0x98000000;
        const u32 base_color = buf[1] >> 8;
        const auto transformed_region_image
            = transform_region_image(*plugin.region_image, flags, base_color);
        const auto transformed_region_data
            = get_rgb_data(transformed_region_image);
        keys[6] = calculate_crc32(transformed_region_data);
        if (plugin.version >= 2390)
            keys[6] += keys[9];
    }

    {
        // this integer represents 100-nanosecond intervals since 01.01.1601
        // (which is Microsoft's equivalent of Unix timestamps).
        const u64 fake_time = ((keys[0] & 0x7FFFFFFFull) << 32) | keys[1];

        u64 n = fake_time / one_millisecond;
        const unsigned int millisecond = n % 1000;
        n /= 1000;
        const unsigned int second = n % 60;
        n /= 60;
        const unsigned int minute = n % 60;
        n /= 60;
        const unsigned int hour = n % 24;
        n /= 24;
        // const unsigned int total_days = n;

        const unsigned int y400 = n / days_per_400_years;
        n -= y400 * days_per_400_years;

        unsigned int y100 = n / days_per_100_years;
        if (y100 == 4)
            y100 = 3;
        n -= y100 * days_per_100_years;

        const unsigned int y4 = n / days_per_4_years;
        n -= y4 * days_per_4_years;

        unsigned int y1 = n / days_per_year;
        if (y1 == 4)
            y1 = 3;
        n -= y1 * days_per_year;

        const unsigned int year = 1601 + y400 * 400 + y100 * 100 + y4 * 4 + y1;
        const bool is_leap = is_leap_year(year);

        unsigned int month = 0;
        while (n >= year_day_acc[is_leap][month])
            month++;

        // const unsigned int day = n - year_day_acc[is_leap][month - 1] + 1;
        // unsigned int day_of_week = (total_days + 1) % 7;

        keys[5] = year | (month << 16);
        keys[7] = hour | (minute << 16);
        keys[8] = second | (millisecond << 16);
    }

    return keys;
}

static void decrypt_initial_160plus(const Plugin &plugin, bstr &data)
{
    const auto keys = get_initial_crypt_keys(plugin, data);
    for (const auto i : util::range(keys.size()))
        data.get<u32>()[1 + i] ^= keys[i];
}

void warc::decrypt_essential(
    const Plugin &plugin, const int warc_version, bstr &data)
{
    if (data.size() < 3)
        return;

    size_t essential_crypt_start = 0;
    auto essential_crypt_end = std::min<size_t>(data.size(), 1024);
    int a, b;
    u64 key_pos_addend1 = 0;
    CustomLcg lcg(data.size());
    if (warc_version > 120)
    {
        a = static_cast<s8>(data[0] ^ (data.size() & 0xFF));
        b = static_cast<s8>(data[1] ^ ((data.size() / 2) & 0xFF));
        if (data.size() != get_max_table_size(plugin, warc_version))
        {
            const size_t index = lcg.next<double>()
                * (static_cast<double>(plugin.logo_data.size()) / 4294967296.0);
            if (warc_version >= 160)
            {
                key_pos_addend1 = lcg.current<u32>() + plugin.logo_data[index];
                key_pos_addend1
                    = get_essential_key_pos_addend1(key_pos_addend1);
                key_pos_addend1 &= 0xFFFFFFF;
                if (essential_crypt_end > 0x80)
                {
                    decrypt_initial_160plus(plugin, data);
                    essential_crypt_start += 0x80;
                }
            }
            else
                throw err::NotSupportedError("WARC <1.6 is not implemented");
        }
    }
    else
        throw err::NotSupportedError("WARC <=1.2 is not implemented");

    const auto tmp = get_lcg_xor(a) * 100000000.0;
    lcg.reset(lcg.current<u32>() ^ static_cast<u32>(tmp));

    double token = 0.0;
    if (a || b)
    {
        token = std::acos(static_cast<double>(a)
            / std::sqrt(static_cast<double>(a * a + b * b)));
        token = token / M_PI * 180.0;
    }
    if (b < 0)
        token = 360.0 - token;

    const u64 key_pos_addend2 = get_essential_key_pos_addend2(token, lcg);
    auto key_pos1 = key_pos_addend1 + key_pos_addend2;
    auto key_pos2 = 0;
    key_pos1 %= plugin.essential_crypt_key.size();

    for (const auto i : util::range(
        essential_crypt_start + 2, essential_crypt_end))
    {
        u8 d = data[i];
        if (warc_version > 120)
            d ^= static_cast<u8>(lcg.next<u32>() / 16777216.0);
        else
            throw err::NotSupportedError("WARC <=1.2 is not implemented");
        d = (d >> 1) | (d << 7);
        d ^= plugin.essential_crypt_key[key_pos1];
        d ^= plugin.essential_crypt_key[key_pos2];
        data[i] = d;
        key_pos1 = d % plugin.essential_crypt_key.size();
        key_pos2 = (key_pos2 + 1) % plugin.essential_crypt_key.size();
    }
}

void warc::decrypt_table_data(
    const Plugin &plugin,
    const int warc_version,
    const size_t table_offset,
    bstr &table_data)
{
    table_data.resize(get_max_table_size(plugin, warc_version));
    decrypt_essential(plugin, warc_version, table_data);

    auto table_data_ptr = table_data.get<u32>();
    const auto table_data_end = table_data.end<const u32>();
    while (table_data_ptr < table_data_end)
        *table_data_ptr++ ^= table_offset;

    if (warc_version >= 170)
    {
        const u8 key = warc_version ^ 0xFF;
        for (const auto i : util::range(8, table_data.size()))
            table_data[i] ^= key;
        table_data = util::pack::zlib_inflate(table_data.substr(8));
    }
    else
        throw err::NotSupportedError("WARC <=1.7 is not implemented");
}

void warc::decrypt_with_flags1(
    const Plugin &plugin, bstr &data, const u32 flags)
{
    if (data.size() < 0x400)
        return;
    const u32 key = 0xECB2F5B2;
    u32 k[4] = { key + 1, key + 4, key + 2, key + 3 };
    if ((flags & 0x202) == 0x202)
    {
        for (const auto i : util::range(0xFF))
        {
            const u32 j = k[3]
                ^ (k[3] << 11)
                ^ k[0]
                ^ ((k[3] ^ (k[3] << 11) ^ (k[0] >> 11)) >> 8);
            k[3] = k[2];
            k[2] = k[1];
            k[1] = k[0];
            k[0] = j;
            const auto idx = j % plugin.flag_crypt.table.size();
            data[i] ^= plugin.flag_crypt.table[idx];
        }
    }
    if ((flags & 0x204) == 0x204)
        data.get<u32>()[0x80] ^= key;
}

void warc::decrypt_with_flags2(
    const Plugin &plugin, bstr &data, const u32 flags)
{
    if (data.size() < 0x200)
        return;
    u16 acc = 0;
    u16 revolve = 0;
    u16 work;
    for (const auto i : util::range(0x100))
    {
        work = data[i];
        acc += work >> 1;
        data[i] = (work >> 1) | revolve;
        revolve = (work & 1) << 7;
    }
    data[0] |= (work & 1) << 7;
    data.get<u32>()[0x41] ^= acc;
}

void warc::decrypt_with_crc(const Plugin &plugin, bstr &data)
{
    if (data.size() < 0x400 || !plugin.crc_crypt.table.size())
        return;

    u32 crc = 0xFFFFFFFF;
    for (const auto i : util::range(0x100))
    {
        crc ^= data[i] << 24;
        for (const auto j : util::range(8))
            crc = (crc << 1) ^ (crc & 0x80000000 ? 0x04C11DB7 : 0);
    }
    for (const auto i : util::range(0x40))
    {
        const auto idx = data.get<u32>()[0x40 + i]
            % plugin.crc_crypt.table.size();
        const u32 src = plugin.crc_crypt.table.get<u32>()[idx / 4];
        const u32 key = src ^ crc;
        data.get<u32>()[0x80 + i] ^= key;
    }
}
