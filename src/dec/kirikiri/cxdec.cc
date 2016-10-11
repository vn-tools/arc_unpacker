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

#include "dec/kirikiri/cxdec.h"
#include "algo/range.h"
#include "err.h"
#include "io/file_byte_stream.h"
#include "io/file_system.h"

using namespace au;
using namespace au::dec::kirikiri;

static const size_t control_block_size = 4096;
static const bstr control_block_magic =
    "\x20\x45\x6E\x63\x72\x79\x70\x74\x69\x6F\x6E\x20\x63\x6F\x6E\x74"_b;

namespace
{
    class KeyDerivationError final : public std::runtime_error
    {
    public:
        KeyDerivationError() : std::runtime_error("") { }
    };

    struct CxdecSettings final
    {
        bstr control_block;
        std::array<size_t, 3> key_derivation_order1;
        std::array<size_t, 8> key_derivation_order2;
        std::array<size_t, 6> key_derivation_order3;
    };

    class KeyDeriver final
    {
    public:
        KeyDeriver(const CxdecSettings &settings);
        u32 derive(u32 seed, u32 parameter);

    private:
        void add_shellcode(const bstr &bytes_s);
        u32 rand();
        u32 derive_for_stage(size_t stage);
        u32 run_first_stage();
        u32 run_stage_strategy_0(size_t stage);
        u32 run_stage_strategy_1(size_t stage);

        const CxdecSettings &settings;
        bstr shellcode;
        u32 seed;
        size_t parameter;
        u32 control_block_addr;
    };
}

static bstr u32_to_string(u32 value)
{
    return bstr(reinterpret_cast<char*>(&value), 4);
}

KeyDeriver::KeyDeriver(const CxdecSettings &settings) : settings(settings)
{
    seed = 0;
    parameter = 0;
    control_block_addr = reinterpret_cast<size_t>(&settings.control_block);
}

u32 KeyDeriver::derive(u32 seed, u32 parameter)
{
    this->seed = seed;
    this->parameter = parameter;

    // What we do: we try to run a code a few times for different "stages".
    // The first one to succeed yields the key.

    // This mechanism of figuring out the valid stage number is really poor,
    // but it's important we do it this way. This is because we initialize the
    // seed only once, and even if we fail to get a number from the given
    // stage, the internal state of randomizer is preserved to the next
    // iteration.

    // Maintaining the randomizer state is essential for the decryption to
    // work.

    for (size_t stage = 5; stage > 0; stage--)
    {
        try
        {
            return derive_for_stage(stage);
        }
        catch (const KeyDerivationError)
        {
            continue;
        }
    }

    throw err::NotSupportedError("Failed to derive the key from the parameter");
}

void KeyDeriver::add_shellcode(const bstr &bytes)
{
    // The execution for current stage must fail when we run code for too long.
    shellcode += bytes;
    if (shellcode.size() > 128)
        throw KeyDerivationError();
}

u32 KeyDeriver::rand()
{
    // This is a modified glibc LCG randomization routine. It is used to make
    // the key as random as possible for each file, which is supposed to
    // maximize confusion.
    const auto old_seed = seed;
    seed = (0x41C64E6D * old_seed) + 12345;
    return seed ^ (old_seed << 16) ^ (old_seed >> 16);
}

u32 KeyDeriver::derive_for_stage(size_t stage)
{
    shellcode = ""_b;

    // push edi, push esi, push ebx, push ecx, push edx
    add_shellcode("\x57\x56\x53\x51\x52"_b);

    // mov edi, dword ptr ss:[esp+18] (esp+18 == parameter)
    add_shellcode("\x86\x7C\x24\x18"_b);

    const auto eax = run_stage_strategy_1(stage);

    // pop edx, pop ecx, pop ebx, pop esi, pop edi
    add_shellcode("\x5A\x59\x5B\x5E\x5F"_b);

    // retn
    add_shellcode("\xC3"_b);

    return eax;
}

u32 KeyDeriver::run_first_stage()
{
    const auto routine_number = settings.key_derivation_order1[rand() % 3];

    u32 eax;
    switch (routine_number)
    {
        case 0:
        {
            // mov eax, rand()
            add_shellcode("\xB8"_b);
            const auto tmp = rand();
            add_shellcode(u32_to_string(tmp));
            eax = tmp;
            break;
        }

        case 1:
            // mov eax, edi
            add_shellcode("\xB8\xC7"_b);
            eax = parameter;
            break;

        case 2:
        {
            // mov esi, &settings.control_block
            add_shellcode("\xBE"_b);
            add_shellcode(u32_to_string(control_block_addr));

            // mov eax, dword ptr ds:[esi+((rand() & 0x3FF) * 4]
            add_shellcode("\x8B\x86"_b);
            const auto pos = (rand() & 0x3FF) * 4;
            add_shellcode(u32_to_string(pos));

            eax = *reinterpret_cast<const u32*>(&settings.control_block[pos]);
            break;
        }

        default:
            throw std::logic_error("Bad routine number");
    }

    return eax;
}

u32 KeyDeriver::run_stage_strategy_0(size_t stage)
{
    if (stage == 1)
        return run_first_stage();

    auto eax = (rand() & 1)
        ? run_stage_strategy_1(stage - 1)
        : run_stage_strategy_0(stage - 1);

    const auto routine_number = settings.key_derivation_order2[rand() % 8];

    switch (routine_number)
    {
        case 0:
            // not eax
            add_shellcode("\xF7\xD0"_b);
            eax ^= 0xFFFFFFFF;
            break;

        case 1:
            // dec eax
            add_shellcode("\x48"_b);
            eax--;
            break;

        case 2:
            // neg eax
            add_shellcode("\xF7\xD8"_b);
            eax = static_cast<u32>(-static_cast<s32>(eax));
            break;

        case 3:
            // inc eax
            add_shellcode("\x40"_b);
            eax++;
            break;

        case 4:
            // mov esi, &settings.control_block
            add_shellcode("\xBE"_b);
            add_shellcode(u32_to_string(control_block_addr));

            // and eax, 3ff
            add_shellcode("\x25\xFF\x03\x00\x00"_b);

            // mov eax, dword ptr ds:[esi+eax*4]
            add_shellcode("\x8B\x04\x86"_b);

            eax = *reinterpret_cast<const u32*>(
                &settings.control_block[(eax & 0x3FF) * 4]);
            break;

        case 5:
        {
            // push ebx
            add_shellcode("\x53"_b);

            // mov ebx, eax
            add_shellcode("\x89\xC3"_b);

            // and ebx, aaaaaaaa
            add_shellcode("\x81\xE3\xAA\xAA\xAA\xAA"_b);

            // and eax, 55555555
            add_shellcode("\x25\x55\x55\x55\x55"_b);

            // shr ebx, 1
            add_shellcode("\xD1\xEB"_b);

            // shl eax, 1
            add_shellcode("\xD1\xE0"_b);

            // or eax, ebx
            add_shellcode("\x09\xD8"_b);

            // pop ebx
            add_shellcode("\x5B"_b);

            auto ebx = eax;
            ebx &= 0xAAAAAAAA;
            eax &= 0x55555555;
            ebx >>= 1;
            eax <<= 1;
            eax |= ebx;
            break;
        }

        case 6:
        {
            // xor eax, rand()
            add_shellcode("\x35"_b);
            const auto tmp = rand();
            add_shellcode(u32_to_string(tmp));

            eax ^= tmp;
            break;
        }

        case 7:
        {
            if (rand() & 1)
            {
                // add eax, rand()
                add_shellcode("\x05"_b);
                const auto tmp = rand();
                add_shellcode(u32_to_string(tmp));

                eax += tmp;
            }
            else
            {
                // sub eax, rand()
                add_shellcode("\x2D"_b);
                const auto tmp = rand();
                add_shellcode(u32_to_string(tmp));

                eax -= tmp;
            }
            break;
        }

        default:
            throw std::logic_error("Bad routine number");
    }

    return eax;
}

u32 KeyDeriver::run_stage_strategy_1(size_t stage)
{
    if (stage == 1)
        return run_first_stage();

    // push ebx
    add_shellcode("\x53"_b);

    auto eax = (rand() & 1)
        ? run_stage_strategy_1(stage - 1)
        : run_stage_strategy_0(stage - 1);

    // mov ebx, eax
    add_shellcode("\x89\xC3"_b);
    const auto ebx = eax;

    eax = (rand() & 1)
        ? run_stage_strategy_1(stage - 1)
        : run_stage_strategy_0(stage - 1);

    const auto routine_number = settings.key_derivation_order3[rand() % 6];
    switch (routine_number)
    {
        case 0:
        {
            // push ecx
            add_shellcode("\x51"_b);

            // mov ecx, ebx
            add_shellcode("\x89\xD9"_b);

            // and ecx, 0f
            add_shellcode("\x83\xE1\x0F"_b);

            // shr eax, cl
            add_shellcode("\xD3\xE8"_b);

            // pop ecx
            add_shellcode("\x59"_b);

            const auto ecx = ebx & 0x0F;
            eax >>= ecx;
            break;
        }

        case 1:
        {
            // push ecx
            add_shellcode("\x51"_b);

            // mov ecx, ebx
            add_shellcode("\x89\xD9"_b);

            // and ecx, 0f
            add_shellcode("\x83\xE1\x0F"_b);

            // shl eax, cl
            add_shellcode("\xD3\xE0"_b);

            // pop ecx
            add_shellcode("\x59"_b);

            const auto ecx = ebx & 0x0F;
            eax <<= ecx;
            break;
        }

        case 2:
            // add eax, ebx
            add_shellcode("\x01\xD8"_b);
            eax += ebx;
            break;

        case 3:
            // neg eax
            add_shellcode("\xF7\xD8"_b);
            // add eax, ebx
            add_shellcode("\x01\xD8"_b);
            eax = ebx - eax;
            break;

        case 4:
            // imul eax, ebx
            add_shellcode("\x0F\xAF\xC3"_b);
            eax *= ebx;
            break;

        case 5:
            // sub eax, ebx
            add_shellcode("\x29\xD8"_b);
            eax -= ebx;
            break;

        default:
            throw std::logic_error("Bad routine number");
    }

    // pop ebx
    add_shellcode("\x5B"_b);

    return eax;
}

static void decrypt_chunk(
    KeyDeriver &key_deriver,
    bstr &data,
    u32 hash,
    size_t base_offset,
    size_t size)
{
    const auto seed = hash & 0x7F;
    hash >>= 7;
    const auto ret0 = key_deriver.derive(seed, hash);
    const auto ret1 = key_deriver.derive(seed, hash ^ 0xFFFFFFFF);

    const auto xor0 = (ret0 >> 8) & 0xFF;
    const auto xor1 = (ret0 >> 16) & 0xFF;
    const auto xor2 = (ret0 & 0xFF) == 0 ? 1 : (ret0 & 0xFF);

    const auto offset0 = ret1 >> 16;
    const auto offset1 = ret1 & 0xFFFF;

    const auto data_ptr = &data[base_offset];

    if (offset0 >= base_offset && offset0 < base_offset + size)
        data_ptr[offset0 - base_offset] ^= xor0;

    if (offset1 >= base_offset && offset1 < base_offset + size)
        data_ptr[offset1 - base_offset] ^= xor1;

    for (const auto i : algo::range(size))
        data_ptr[i] ^= xor2;
}

static bstr find_control_block(const io::path &path)
{
    const auto dir = path.parent();
    for (const auto &path : io::recursive_directory_range(dir))
    {
        if (!io::is_regular_file(path))
            continue;

        const auto fn = path.str();
        if (fn.find(".tpm") != fn.size() - 4)
            continue;

        io::FileByteStream tmp_stream(path, io::FileMode::Read);
        const auto content = tmp_stream.read_to_eof();
        const auto pos = content.find(control_block_magic);
        if (pos == bstr::npos)
        {
            throw err::CorruptDataError(
                "TPM file found, but without control block");
        }

        if (pos + control_block_size > content.size())
            throw err::CorruptDataError("Control block found, but truncated");

        return content.substr(pos, control_block_size);
    }

    throw err::FileNotFoundError("TPM file not found");
}

Xp3Plugin au::dec::kirikiri::create_cxdec_plugin(
    const u16 key1,
    const u16 key2,
    const std::array<size_t, 3> key_derivation_order1,
    const std::array<size_t, 8> key_derivation_order2,
    const std::array<size_t, 6> key_derivation_order3,
    const bstr &control_block)
{
    Xp3Plugin plugin;
    plugin.create_decrypt_func = [=](const io::path &arc_path)
        -> std::function<void(bstr &, u32)> // fixes crash in clang
    {
        CxdecSettings settings;
        settings.control_block = control_block.empty()
            ? find_control_block(arc_path)
            : control_block;
        settings.key_derivation_order1 = key_derivation_order1;
        settings.key_derivation_order2 = key_derivation_order2;
        settings.key_derivation_order3 = key_derivation_order3;

        return [=](bstr &data, u32 adlr_key)
        {
            KeyDeriver key_deriver(settings);
            const auto hash1 = adlr_key;
            const auto hash2 = (adlr_key >> 16) ^ adlr_key;
            const auto offset1 = 0;
            const auto offset2 = std::min<size_t>(
                data.size(), (adlr_key & key1) + key2);
            decrypt_chunk(key_deriver, data, hash1, offset1, offset2);
            decrypt_chunk(
                key_deriver, data, hash2, offset2, data.size() - offset2);
        };
    };
    return plugin;
}
