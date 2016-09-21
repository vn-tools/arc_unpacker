#include "dec/dogenzaka/a_file_decoder.h"
#include <algorithm>
#include "algo/crypt/sha1.h"
#include "algo/range.h"
#include <array>

using namespace au;
using namespace au::dec::dogenzaka;

static const bstr magic = "\x1A\xF6\xF7\xC4"_b; // PNG file
static const bstr password = "Hlk9D28p"_b;

static bstr transform_rc4(const bstr &input, const bstr &key)
{
    std::array<u8, 256> state;
    size_t i1, i2;

    for (const auto i : algo::range(state.size()))
        state[i] = i;

    i1 = i2 = 0;
    for (const auto i : algo::range(state.size()))
    {
        i2 = (key[i1] + state[i] + i2) % state.size();
        i1 = (i1 + 1) % key.size();
        std::swap(state[i], state[i2]);
    }

    i1 = i2 = 0;
    bstr output(input.size());
    for (const auto i : algo::range(input.size()))
    {
        i1 = (i1 + 1) % state.size();
        i2 = (state[i1] + i2) % state.size();

        std::swap(state[i1], state[i2]);
        output[i] = input[i] ^ state[(state[i1] + state[i2]) % state.size()];
    }

    return output;
}

static const bstr get_key(const bstr &password)
{
    return algo::crypt::sha1(password).substr(0, 16);
}

bool AFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<io::File> AFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto key = get_key(password);
    const bstr data = transform_rc4(
        input_file.stream.seek(0).read_to_eof(), key);
    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<AFileDecoder>("dogenzaka/a");
