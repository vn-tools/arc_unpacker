#include <stdexcept>
#include "formats/arc/xp3_archive/xp3_filter_cxdec.h"

namespace
{
    class KeyDerivationError : public std::runtime_error
    {
    public:
        KeyDerivationError() : std::runtime_error("")
        {
        }
    };

    // A class used to derive the nontrivial key in CXDEC based encryption.
    class KeyDeriver
    {
    public:
        KeyDeriver(const Xp3FilterCxdecSettings &settings) : settings(settings)
        {
            seed = 0;
            parameter = 0;
            encryption_block_addr
                = reinterpret_cast<size_t>(settings.encryption_block);
        }

        uint32_t derive(uint32_t seed, uint32_t parameter)
        {
            this->seed = seed;
            this->parameter = parameter;

            // What we do: we try to run a code a few times for different
            // "stages".  The first one to succeed yields the key.

            // This mechanism of figuring out the valid stage number is really
            // crappy, but it's important we do it this way. This is because we
            // initialize the seed only once, and even if we fail to get a
            // number from the given stage, the internal state of randomizer is
            // preserved to the next iteration.

            // Maintaining the randomizer state is essential for the decryption
            // to work.

            for (size_t stage = 5; stage >= 1; stage --)
            {
                try
                {
                    return derive_for_stage(stage);
                }
                catch (KeyDerivationError)
                {
                    continue;
                }
            }

            throw std::runtime_error(
                "Failed to derive the key from the parameter");
        }

    private:
        const Xp3FilterCxdecSettings &settings;
        std::string shellcode;
        uint32_t seed;
        size_t parameter;
        unsigned long encryption_block_addr;

        // The execution for current stage must fail when we run code for too
        // long.
        void add_shellcode(const char *bytes, size_t count)
        {
            shellcode += std::string(bytes, count);
            if (shellcode.size() > 128)
                throw KeyDerivationError();
        }

        // This is a modified glibc LCG randomization routine. It is used to
        // make the key as random as possible for each file, which is supposed
        // to maximize confusion.
        uint32_t rand()
        {
            uint32_t old_seed = seed;
            seed = (0x41C64E6D * old_seed) + 12345;
            return seed ^ (old_seed << 16) ^ (old_seed >> 16);
        }

        uint32_t derive_for_stage(size_t stage)
        {
            shellcode = "";

            // push edi, push esi, push ebx, push ecx, push edx
            add_shellcode("\x57\x56\x53\x51\x52", 5);

            // mov edi, dword ptr ss:[esp+18] (esp+18 == parameter)
            add_shellcode("\x86\x7c\x24\x18", 4);

            uint32_t eax = run_stage_strategy_1(stage);

            // pop edx, pop ecx, pop ebx, pop esi, pop edi
            add_shellcode("\x5a\x59\x5b\x5e\x5f", 5);

            // retn
            add_shellcode("\xc3", 1);

            return eax;
        }

        uint32_t run_first_stage()
        {
            size_t routine_number = settings.key_derivation_order1[rand() % 3];

            uint32_t eax;
            switch (routine_number)
            {
                case 0:
                {
                    // mov eax, rand()
                    add_shellcode("\xb8", 1);
                    uint32_t tmp = rand();
                    add_shellcode(reinterpret_cast<char*>(&tmp), 4);
                    eax = tmp;
                    break;
                }

                case 1:
                    // mov eax, edi
                    add_shellcode("\xb8\xc7", 2);
                    eax = parameter;
                    break;

                case 2:
                {
                    // mov esi, &encryption_block
                    add_shellcode("\xbe", 1);
                    add_shellcode(reinterpret_cast<char*>(
                        &encryption_block_addr), 4);

                    // mov eax, dword ptr ds:[esi+((rand() & 0x3ff) * 4]
                    add_shellcode("\x8b\x86", 2);
                    uint32_t pos = (rand() & 0x3ff) * 4;
                    add_shellcode(reinterpret_cast<char*>(&pos), 4);

                    eax = *reinterpret_cast<const uint32_t*>(
                        &settings.encryption_block[pos]);
                    break;
                }

                default:
                    throw std::runtime_error("Bad routine number");
            }

            return eax;
        }

        uint32_t run_stage_strategy_0(size_t stage)
        {
            if (stage == 1)
                return run_first_stage();

            uint32_t eax = (rand() & 1)
                ? run_stage_strategy_1(stage - 1)
                : run_stage_strategy_0(stage - 1);

            size_t routine_number = settings.key_derivation_order2[rand() % 8];

            switch (routine_number)
            {
                case 0:
                    // not eax
                    add_shellcode("\xf7\xd0", 2);
                    eax ^= 0xffffffff;
                    break;

                case 1:
                    // dec eax
                    add_shellcode("\x48", 1);
                    eax -= 1;
                    break;

                case 2:
                    // neg eax
                    add_shellcode("\xf7\xd8", 2);
                    eax = -(signed)eax;
                    break;

                case 3:
                    // inc eax
                    add_shellcode("\x40", 1);
                    eax += 1;
                    break;

                case 4:
                    // mov esi, &encryption_block
                    add_shellcode("\xbe", 1);
                    add_shellcode(reinterpret_cast<char*>(
                        &encryption_block_addr), 4);

                    // and eax, 3ff
                    add_shellcode("\x25\xff\x03\x00\x00", 5);

                    // mov eax, dword ptr ds:[esi+eax*4]
                    add_shellcode("\x8b\x04\x86", 3);

                    eax = *reinterpret_cast<const uint32_t*>(
                        &settings.encryption_block[(eax & 0x3ff) * 4]);
                    break;

                case 5:
                {
                    // push ebx
                    add_shellcode("\x53", 1);

                    // mov ebx, eax
                    add_shellcode("\x89\xc3", 2);

                    // and ebx, aaaaaaaa
                    add_shellcode("\x81\xe3\xaa\xaa\xaa\xaa", 6);

                    // and eax, 55555555
                    add_shellcode("\x25\x55\x55\x55\x55", 5);

                    // shr ebx, 1
                    add_shellcode("\xd1\xeb", 2);

                    // shl eax, 1
                    add_shellcode("\xd1\xe0", 2);

                    // or eax, ebx
                    add_shellcode("\x09\xd8", 2);

                    // pop ebx
                    add_shellcode("\x5b", 1);

                    uint32_t ebx = eax;
                    ebx &= 0xaaaaaaaa;
                    eax &= 0x55555555;
                    ebx >>= 1;
                    eax <<= 1;
                    eax |= ebx;
                    break;
                }

                case 6:
                {
                    // xor eax, rand()
                    add_shellcode("\x35", 1);
                    uint32_t tmp = rand();
                    add_shellcode(reinterpret_cast<char*>(&tmp), 4);

                    eax ^= tmp;
                    break;
                }

                case 7:
                {
                    if (rand() & 1)
                    {
                        // add eax, rand()
                        add_shellcode("\x05", 1);
                        uint32_t tmp = rand();
                        add_shellcode(reinterpret_cast<char*>(&tmp), 4);

                        eax += tmp;
                    }
                    else
                    {
                        // sub eax, rand()
                        add_shellcode("\x2d", 1);
                        uint32_t tmp = rand();
                        add_shellcode(reinterpret_cast<char*>(&tmp), 4);

                        eax -= tmp;
                    }
                    break;
                }

                default:
                    throw std::runtime_error("Bad routine number");
            }

            return eax;
        }

        uint32_t run_stage_strategy_1(size_t stage)
        {
            if (stage == 1)
                return run_first_stage();

            // push ebx
            add_shellcode("\x53", 1);

            uint32_t eax = (rand() & 1)
                ? run_stage_strategy_1(stage - 1)
                : run_stage_strategy_0(stage - 1);

            // mov ebx, eax
            add_shellcode("\x89\xc3", 2);
            uint32_t ebx = eax;

            eax = (rand() & 1)
                ? run_stage_strategy_1(stage - 1)
                : run_stage_strategy_0(stage - 1);

            size_t routine_number = settings.key_derivation_order3[rand() % 6];
            switch (routine_number)
            {
                case 0:
                {
                    // push ecx
                    add_shellcode("\x51", 1);

                    // mov ecx, ebx
                    add_shellcode("\x89\xd9", 2);

                    // and ecx, 0f
                    add_shellcode("\x83\xe1\x0f", 3);

                    // shr eax, cl
                    add_shellcode("\xd3\xe8", 2);

                    // pop ecx
                    add_shellcode("\x59", 1);

                    uint8_t ecx = ebx & 0x0f;
                    eax >>= ecx;
                    break;
                }

                case 1:
                {
                    // push ecx
                    add_shellcode("\x51", 1);

                    // mov ecx, ebx
                    add_shellcode("\x89\xd9", 2);

                    // and ecx, 0f
                    add_shellcode("\x83\xe1\x0f", 3);

                    // shl eax, cl
                    add_shellcode("\xd3\xe0", 2);

                    // pop ecx
                    add_shellcode("\x59", 1);

                    uint8_t ecx = ebx & 0x0f;
                    eax <<= ecx;
                    break;
                }

                case 2:
                    // add eax, ebx
                    add_shellcode("\x01\xd8", 2);
                    eax += ebx;
                    break;

                case 3:
                    // neg eax
                    add_shellcode("\xf7\xd8", 2);
                    // add eax, ebx
                    add_shellcode("\x01\xd8", 2);
                    eax = ebx - eax;
                    break;

                case 4:
                    // imul eax, ebx
                    add_shellcode("\x0f\xaf\xc3", 3);
                    eax *= ebx;
                    break;

                case 5:
                    // sub eax, ebx
                    add_shellcode("\x29\xd8", 2);
                    eax -= ebx;
                    break;

                default:
                    throw std::runtime_error("Bad routine number");
            }

            // pop ebx
            add_shellcode("\x5b", 1);

            return eax;
        }
    };

    void decrypt_chunk(
        KeyDeriver &key_deriver,
        IO &io,
        uint32_t hash,
        size_t base_offset,
        size_t length)
    {
        uint32_t seed = hash & 0x7f;
        hash >>= 7;
        uint32_t ret0 = key_deriver.derive(seed, hash);
        uint32_t ret1 = key_deriver.derive(seed, hash ^ 0xffffffff);

        uint8_t xor0 = (ret0 >> 8) & 0xff;
        uint8_t xor1 = (ret0 >> 16) & 0xff;
        uint8_t xor2 = ret0 & 0xff;
        if (xor2 == 0)
            xor2 = 1;

        size_t offset0 = ret1 >> 16;
        size_t offset1 = ret1 & 0xffff;

        std::unique_ptr<char> data(new char[length]);
        char *ptr = data.get();
        io.seek(base_offset);
        io.read(ptr, length);

        if (offset0 >= base_offset && offset0 < base_offset + length)
            ptr[offset0 - base_offset] ^= xor0;

        if (offset1 >= base_offset && offset1 < base_offset + length)
            ptr[offset1 - base_offset] ^= xor1;

        for (size_t i = 0; i < length; i ++)
            ptr[i] ^= xor2;

        io.seek(base_offset);
        io.write(ptr, length);
    }
}

struct Xp3FilterCxdec::Internals
{
    KeyDeriver key_deriver;
    const Xp3FilterCxdecSettings &settings;

    Internals(const Xp3FilterCxdecSettings &settings)
        : key_deriver(settings), settings(settings)
    {
    }

    ~Internals()
    {
    }
};

Xp3FilterCxdec::Xp3FilterCxdec(Xp3FilterCxdecSettings &settings)
    : internals(new Internals(settings))
{
}

Xp3FilterCxdec::~Xp3FilterCxdec()
{
}

void Xp3FilterCxdec::decode(VirtualFile &file, uint32_t encryption_key) const
{
    uint32_t hash = encryption_key;
    uint32_t key = (hash & internals->settings.key1) + internals->settings.key2;

    size_t size = file.io.size() > key ? key : file.io.size();

    decrypt_chunk(internals->key_deriver, file.io, hash, 0, size);
    size_t offset = size;
    size = file.io.size() - offset;
    hash = (hash >> 16) ^ hash;
    decrypt_chunk(internals->key_deriver, file.io, hash, offset, size);
}
