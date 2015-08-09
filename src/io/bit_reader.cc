#include <stdexcept>
#include "io/bit_reader.h"

using namespace au;
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
        bstr buffer;
        size_t bytes_left;
        const u8 *ptr;

    public:
        BufferBasedReader(const bstr &buffer);
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


BufferBasedReader::BufferBasedReader(const bstr &buffer)
    : buffer(buffer), bytes_left(buffer.size())
{
    ptr = this->buffer.get<const u8>();
}

bool BufferBasedReader::eof()
{
    return bytes_left == 0;
}

u8 BufferBasedReader::fetch_byte()
{
    --bytes_left;
    return *ptr++;
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

BitReader::BitReader(const bstr &buffer)
    : p(new Priv(std::unique_ptr<Reader>(new BufferBasedReader(buffer))))
{
}

BitReader::BitReader(const char *buffer, size_t size)
    : p(new Priv(
        std::unique_ptr<Reader>(
            new BufferBasedReader(
                bstr(buffer, size)))))
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
