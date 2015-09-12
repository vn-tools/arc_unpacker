#include "err.h"
#include "io/bit_reader.h"

using namespace au;
using namespace au::io;

namespace
{
    struct Reader
    {
        Reader();
        virtual ~Reader();
        virtual bool eof() const = 0;
        virtual size_t size() const = 0;
        virtual u8 fetch_byte() = 0;
    };

    struct BufferBasedReader final : public Reader
    {
        bstr buffer;
        size_t bytes_left;
        const u8 *ptr;

        BufferBasedReader(const bstr &buffer);
        inline bool eof() const override;
        inline size_t size() const override;
        inline u8 fetch_byte() override;
    };

    struct IoBasedReader final : public Reader
    {
        IO &io;

        IoBasedReader(IO &io);
        inline bool eof() const override;
        inline size_t size() const override;
        inline u8 fetch_byte() override;
    };
}

Reader::Reader()
{
}

Reader::~Reader()
{
}

BufferBasedReader::BufferBasedReader(const bstr &buffer)
    : buffer(buffer), bytes_left(buffer.size())
{
    ptr = this->buffer.get<const u8>();
}

inline bool BufferBasedReader::eof() const
{
    return bytes_left == 0;
}

inline size_t BufferBasedReader::size() const
{
    return buffer.size();
}

inline u8 BufferBasedReader::fetch_byte()
{
    --bytes_left;
    return *ptr++;
}

IoBasedReader::IoBasedReader(IO &io) : io(io)
{
}

inline bool IoBasedReader::eof() const
{
    return io.tell() >= io.size();
}

inline size_t IoBasedReader::size() const
{
    return io.size();
}

inline u8 IoBasedReader::fetch_byte()
{
    return io.read_u8();
}

struct BitReader::Priv
{
    std::unique_ptr<Reader> reader;
    u32 shift;
    u32 value;
    u32 pos;

    Priv(std::unique_ptr<Reader> reader);
    ~Priv();
    inline u32 tell() const;
    inline u32 get(size_t n, bool use_exceptions);
    bool eof() const;
    size_t size() const;
};

BitReader::Priv::Priv(std::unique_ptr<Reader> reader)
    : reader(std::move(reader))
{
    value = 0;
    pos = 0;
    shift = 8;
}

BitReader::Priv::~Priv()
{
}

inline u32 BitReader::Priv::tell() const
{
    return pos;
}

inline u32 BitReader::Priv::get(size_t n, bool use_exceptions)
{
    if (n > 32)
        throw err::NotSupportedError("Too many bits");

    auto mask = ((1ull << n) - 1);

    pos += n;
    shift += n;
    while (shift > 8)
    {
        value <<= 8;
        if (reader->eof())
        {
            if (use_exceptions)
                throw err::IoError("Trying to read bits beyond EOF");
        }
        else
            value |= reader->fetch_byte();
        shift -= 8;
    }

    return (value >> ((32 - shift) & 7)) & mask;
}

inline bool BitReader::Priv::eof() const
{
    return pos % 8 == 0 && reader->eof();
}

inline size_t BitReader::Priv::size() const
{
    return reader->size() * 8;
}


BitReader::BitReader(IO &io)
    : p(new Priv(std::unique_ptr<Reader>(new IoBasedReader(io))))
{
}

BitReader::BitReader(const bstr &buffer)
    : p(new Priv(std::unique_ptr<Reader>(new BufferBasedReader(buffer))))
{
}

BitReader::BitReader(const char *buffer, size_t size)
    : p(new Priv(std::unique_ptr<Reader>(new BufferBasedReader(
                bstr(buffer, size)))))
{
}

BitReader::~BitReader()
{
}

unsigned int BitReader::get(size_t n)
{
    return p->get(n, true);
}

unsigned int BitReader::try_get(size_t n)
{
    return p->get(n, false);
}

bool BitReader::eof() const
{
    return p->eof();
}

size_t BitReader::size() const
{
    return p->size();
}

size_t BitReader::tell() const
{
    return p->tell();
}
