#include "io/bit_reader.h"
#include "err.h"

using namespace au;
using namespace au::io;

namespace
{
    class Reader
    {
    public:
        Reader();
        virtual ~Reader();
        virtual bool eof() const = 0;
        virtual size_t size() const = 0;
        virtual u8 fetch_byte() = 0;
        virtual void seek(size_t) = 0;
    };

    class BufferBasedReader final : public Reader
    {
    public:
        BufferBasedReader(const bstr &buffer);
        inline bool eof() const override;
        inline size_t size() const override;
        inline u8 fetch_byte() override;
        inline void seek(size_t pos) override;

    private:
        bstr buffer;
        size_t bytes_left;
        const u8 *ptr;
    };

    class IoBasedReader final : public Reader
    {
    public:
        IoBasedReader(IO &io);
        inline bool eof() const override;
        inline size_t size() const override;
        inline u8 fetch_byte() override;
        inline void seek(size_t pos) override;

    private:
        IO &io;
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

inline void BufferBasedReader::seek(size_t pos)
{
    ptr = &buffer.get<const u8>()[pos];
    bytes_left = buffer.size() - pos;
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

inline void IoBasedReader::seek(size_t pos)
{
    io.seek(pos);
}

inline u8 IoBasedReader::fetch_byte()
{
    return io.read_u8();
}

struct BitReader::Priv final
{
    Priv(std::unique_ptr<Reader> reader);
    ~Priv();
    inline u32 tell() const;
    inline u32 get(size_t n);
    bool eof() const;
    size_t size() const;
    void seek(size_t pos);

    std::unique_ptr<Reader> reader;
    u32 shift;
    u64 value;
    u32 pos;
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

inline void BitReader::Priv::seek(size_t new_pos)
{
    if (new_pos > size())
        throw err::IoError("Seeking beyond EOF");
    pos = (new_pos / 32) * 32;
    shift = 8;
    value = 0;
    reader->seek(pos / 8);
    get(new_pos % 32);
}

inline u32 BitReader::Priv::get(size_t n)
{
    if (n > 32)
        throw err::NotSupportedError("Too many bits");

    auto mask = ((1ull << n) - 1);

    pos += n;
    shift += n;
    while (shift > 8)
    {
        if (reader->eof())
        {
            seek(pos - n);
            throw err::IoError("Trying to read bits beyond EOF");
        }
        value <<= 8;
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

BitReader::BitReader(IO &io) : p(new Priv(std::make_unique<IoBasedReader>(io)))
{
}

BitReader::BitReader(const bstr &buffer)
    : p(new Priv(std::make_unique<BufferBasedReader>(buffer)))
{
}

BitReader::BitReader(const char *buffer, size_t size)
    : p(new Priv(std::make_unique<BufferBasedReader>(bstr(buffer, size))))
{
}

BitReader::~BitReader()
{
}

unsigned int BitReader::get(size_t n)
{
    return p->get(n);
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

void BitReader::seek(size_t pos)
{
    return p->seek(pos);
}

void BitReader::skip(int offset)
{
    return p->seek(p->tell() + offset);
}
