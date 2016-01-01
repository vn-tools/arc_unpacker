#include "dec/borland/tpf0_decoder.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::borland;

static const auto magic = "TPF0"_b;

namespace
{
    struct delphi_f80 final
    {
        u64 mantissa : 64;
        u32 exponent : 15;
        u8 sign : 1;
    };

    struct ieee754_f64 final
    {
        u64 mantissa : 52;
        u32 exponent : 11;
        u8 sign : 1;
    };
}

static algo::any read_value(io::IStream &input_stream)
{
    const auto type = input_stream.read_u8();
    if (type == 0)
        throw err::CorruptDataError("Zero not supported in this context");

    if (type == 2)
        return static_cast<int>(input_stream.read_u8());

    if (type == 3)
        return static_cast<int>(input_stream.read_u16_le());

    if (type == 5)
    {
        const auto value_f80 = *input_stream.read(10).get<delphi_f80>();
        ieee754_f64 value_f64;
        value_f64.mantissa = value_f80.mantissa >> 12;
        value_f64.exponent = value_f80.exponent >> 4;
        value_f64.sign = value_f80.sign;
        const auto alias = reinterpret_cast<const char*>(&value_f64);
        return *reinterpret_cast<const f64*>(alias);
    }

    if (type == 6 || type == 7)
        return input_stream.read(input_stream.read_u8()).str();

    if (type == 8)
        return true;

    if (type == 9)
        return true;

    if (type == 10)
        return input_stream.read(input_stream.read_u32_le());

    if (type == 11)
    {
        std::vector<std::string> ret;
        while (true)
        {
            const auto size = input_stream.read_u8();
            if (size == 0)
                break;
            ret.push_back(input_stream.read(size).str());
        }
        return ret;
    }

    if (type == 18)
    {
        const auto size = input_stream.read_u32_le();
        return algo::utf16_to_utf8(input_stream.read(size * 2)).str();
    }

    throw err::NotSupportedError(
        algo::format("Unknown value type: %d\n", type));
}

static std::unique_ptr<Tpf0Structure> read_structure(io::IStream &input_stream)
{
    const auto type_size = input_stream.read_u8();
    auto s = std::make_unique<Tpf0Structure>();
    if (type_size == 0)
        return nullptr;
    s->type = input_stream.read(type_size).str();
    s->name = input_stream.read(input_stream.read_u8()).str();
    while (true)
    {
        const auto key_size = input_stream.read_u8();
        if (!key_size)
            break;
        const auto key = input_stream.read(key_size).str();
        s->properties[key] = read_value(input_stream);
    }
    while (true)
    {
        auto child = read_structure(input_stream);
        if (child == nullptr)
            break;
        s->children.push_back(std::move(child));
    }
    return s;
}

std::unique_ptr<Tpf0Structure> Tpf0Decoder::decode(const bstr &input) const
{
    io::MemoryStream input_stream(input);
    if (input_stream.read(magic.size()) != magic)
        throw err::RecognitionError();
    return read_structure(input_stream);
}
