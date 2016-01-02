#include "dec/borland/tpf0_decoder.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::borland;

static const auto magic = "TPF0"_b;

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
        const auto hi = input_stream.read_u64_le();
        const auto lo = input_stream.read_u16_le();
        const u64 f80_sign     = lo >> 15;  // 1
        const u64 f80_exponent = lo & ~15;  // 15
        const u64 f80_mantissa = hi;        // 64
        const u64 f64_sign     = f80_sign;              // 1
        const u64 f64_exponent = f80_exponent >> 4;     // 11
        const u64 f64_mantissa = f80_mantissa >> 12;    // 52
        const u64 packed_f64
            = (f64_sign << 63)
            | (f64_exponent << 52)
            | (f64_mantissa << 0);
        const auto alias = reinterpret_cast<const char*>(&packed_f64);
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
