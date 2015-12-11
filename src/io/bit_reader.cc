#include "io/bit_reader.h"
#include "err.h"

using namespace au;
using namespace au::io;

static const size_t buffer_size = 8192;
static u64 masks[33] =
{
    0b00000000'00000000'00000000'00000000,
    0b00000000'00000000'00000000'00000001,
    0b00000000'00000000'00000000'00000011,
    0b00000000'00000000'00000000'00000111,
    0b00000000'00000000'00000000'00001111,
    0b00000000'00000000'00000000'00011111,
    0b00000000'00000000'00000000'00111111,
    0b00000000'00000000'00000000'01111111,
    0b00000000'00000000'00000000'11111111,
    0b00000000'00000000'00000001'11111111,
    0b00000000'00000000'00000011'11111111,
    0b00000000'00000000'00000111'11111111,
    0b00000000'00000000'00001111'11111111,
    0b00000000'00000000'00011111'11111111,
    0b00000000'00000000'00111111'11111111,
    0b00000000'00000000'01111111'11111111,
    0b00000000'00000000'11111111'11111111,
    0b00000000'00000001'11111111'11111111,
    0b00000000'00000011'11111111'11111111,
    0b00000000'00000111'11111111'11111111,
    0b00000000'00001111'11111111'11111111,
    0b00000000'00011111'11111111'11111111,
    0b00000000'00111111'11111111'11111111,
    0b00000000'01111111'11111111'11111111,
    0b00000000'11111111'11111111'11111111,
    0b00000001'11111111'11111111'11111111,
    0b00000011'11111111'11111111'11111111,
    0b00000111'11111111'11111111'11111111,
    0b00001111'11111111'11111111'11111111,
    0b00011111'11111111'11111111'11111111,
    0b00111111'11111111'11111111'11111111,
    0b01111111'11111111'11111111'11111111,
    0b11111111'11111111'11111111'11111111,
};

namespace
{
    class IReader
    {
    public:
        virtual ~IReader() { }
        virtual bool eof() const = 0;
        virtual size_t size() const = 0;
        virtual u8 fetch_byte() = 0;
        virtual bstr fetch_buffer() = 0;
        virtual void seek(const size_t pos) = 0;
    };

    class BufferBasedReader final : public IReader
    {
    public:
        BufferBasedReader(const bstr &source);
        inline bool eof() const override;
        inline size_t size() const override;
        inline u8 fetch_byte() override;
        inline bstr fetch_buffer() override;
        inline void seek(const size_t pos) override;

    private:
        bstr source;
        const u8 *ptr, *end;
    };

    class StreamBasedReader final : public IReader
    {
    public:
        StreamBasedReader(Stream &stream);
        inline bool eof() const override;
        inline size_t size() const override;
        inline u8 fetch_byte() override;
        inline bstr fetch_buffer() override;
        inline void seek(const size_t pos) override;

    private:
        Stream &stream;
    };
}

BufferBasedReader::BufferBasedReader(const bstr &source) : source(source)
{
    ptr = this->source.get<const u8>();
    end = this->source.end<const u8>();
}

inline bool BufferBasedReader::eof() const
{
    return ptr >= end;
}

inline size_t BufferBasedReader::size() const
{
    return source.size();
}

inline void BufferBasedReader::seek(const size_t pos)
{
    ptr = &source.get<const u8>()[pos];
}

inline u8 BufferBasedReader::fetch_byte()
{
    return *ptr++;
}

inline bstr BufferBasedReader::fetch_buffer()
{
    auto chunk_size = std::min<size_t>(end - ptr, buffer_size);
    auto buffer = bstr(ptr, chunk_size);
    ptr += chunk_size;
    return buffer;
}

StreamBasedReader::StreamBasedReader(Stream &stream) : stream(stream)
{
}

inline bool StreamBasedReader::eof() const
{
    return stream.tell() >= stream.size();
}

inline size_t StreamBasedReader::size() const
{
    return stream.size();
}

inline void StreamBasedReader::seek(const size_t pos)
{
    stream.seek(pos);
}

inline u8 StreamBasedReader::fetch_byte()
{
    return stream.read_u8();
}

inline bstr StreamBasedReader::fetch_buffer()
{
    // the eventual position within the user-supplied Stream might be important
    // to the caller, so we can't buffer anything more than one byte.
    return stream.read(1);
}

struct BitReader::Priv final
{
    Priv(std::unique_ptr<IReader> reader);
    ~Priv();
    unsigned int get(const size_t n);
    void seek(const size_t new_pos);
    size_t size() const;

    std::unique_ptr<IReader> reader;
    bstr buffer;
    const u8 *buffer_ptr, *buffer_end;
    u64 value;
    u32 pos;
    u32 shift;
};

BitReader::Priv::Priv(std::unique_ptr<IReader> reader) :
    reader(std::move(reader)),
    buffer_ptr(nullptr),
    buffer_end(nullptr),
    value(0),
    pos(0),
    shift(8)
{
}

BitReader::Priv::~Priv()
{
}

unsigned int BitReader::Priv::get(const size_t n)
{
    if (n > 32)
        throw err::NotSupportedError("Too many bits");

    pos += n;
    shift += n;
    while (shift > 8)
    {
        if (buffer_ptr >= buffer_end)
        {
            if (reader->eof())
            {
                seek(pos - n);
                throw err::EofError();
            }
            buffer = reader->fetch_buffer();
            buffer_ptr = buffer.get<const u8>();
            buffer_end = buffer.end<const u8>();
        }
        value <<= 8;
        value |= *buffer_ptr++;
        shift -= 8;
    }

    return (value >> (8 - shift)) & masks[n];
}

void BitReader::Priv::seek(const size_t new_pos)
{
    if (new_pos > size())
        throw err::EofError();
    pos = (new_pos / 32) * 32;
    shift = 8;
    value = 0;
    reader->seek(pos / 8);
    buffer = ""_b;
    buffer_ptr = buffer_end = buffer.end<const u8>();
    get(new_pos % 32);
}

size_t BitReader::Priv::size() const
{
    return reader->size() * 8;
}

BitReader::BitReader(Stream &stream)
    : p(new Priv(std::make_unique<StreamBasedReader>(stream)))
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

unsigned int BitReader::get(const size_t n)
{
    return p->get(n);
}

bool BitReader::eof() const
{
    return p->pos == size();
}

size_t BitReader::size() const
{
    return p->size();
}

size_t BitReader::tell() const
{
    return p->pos;
}

void BitReader::seek(const size_t new_pos)
{
    p->seek(new_pos);
}

void BitReader::skip(int offset)
{
    return seek(tell() + offset);
}
