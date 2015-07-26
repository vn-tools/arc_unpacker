#include <stdexcept>
#include "io/bit_reader.h"

using namespace au::io;

namespace
{
    class Reader
    {
    private:
        u8 mask;
        u8 value;
        virtual bool eof() = 0;
        virtual u8 fetch_byte() = 0;

    public:
        Reader();
        bool get(bool exception);
        int getn(size_t n, bool exception);
    };

    class BufferBasedReader : public Reader
    {
    private:
        const u8 *buffer;
        size_t buffer_size;

    public:
        BufferBasedReader(const u8 *buffer, size_t buffer_size);
        bool eof() override;
        u8 fetch_byte() override;
    };

    class IoBasedReader : public Reader
    {
    private:
        IO &io;

    public:
        IoBasedReader(IO &io);
        bool eof() override;
        u8 fetch_byte() override;
    };
}

Reader::Reader()
{
    mask = 0;
    value = 0;
}

bool Reader::get(bool exception)
{
    mask >>= 1;
    if (mask == 0x00)
    {
        if (eof())
        {
            if (!exception)
                return 0;
            throw std::runtime_error("Trying to read bits beyond EOF");
        }

        mask = 0x80;
        value = fetch_byte();
    }
    return (value & mask) != 0;
}

int Reader::getn(size_t n, bool exception)
{
    if (n > 32)
        throw std::runtime_error("Too many bits");

    unsigned int value = 0;
    while (n--)
    {
        value <<= 1;
        value |= static_cast<int>(get(exception));
    }
    return value;
}


BufferBasedReader::BufferBasedReader(const u8 *buffer, size_t buffer_size)
    : buffer(buffer), buffer_size(buffer_size)
{
}

bool BufferBasedReader::eof()
{
    return buffer_size == 0;
}

u8 BufferBasedReader::fetch_byte()
{
    --buffer_size;
    return *buffer++;
}

IoBasedReader::IoBasedReader(IO &io) : io(io)
{
}

bool IoBasedReader::eof()
{
    return io.tell() >= io.size();
}

u8 IoBasedReader::fetch_byte()
{
    return io.read_u8();
}

struct BitReader::Priv
{
    std::unique_ptr<Reader> reader;

    Priv(std::unique_ptr<Reader> reader) : reader(std::move(reader))
    {
    }

    ~Priv()
    {
    }
};

BitReader::BitReader(IO &io)
    : p(
        new Priv(
            std::unique_ptr<Reader>(
                new IoBasedReader(io))))
{
}

BitReader::BitReader(const char *buffer, size_t buffer_size)
    : p(
        new Priv(
            std::unique_ptr<Reader>(
                new BufferBasedReader(
                    reinterpret_cast<const u8*>(buffer), buffer_size))))
{
}

BitReader::~BitReader()
{
}

unsigned int BitReader::get(size_t n)
{
    return p->reader->getn(n, true);
}

unsigned int BitReader::try_get(size_t n)
{
    return p->reader->getn(n, false);
}
