#ifndef IO_IO_H
#define IO_IO_H
#include <cstdint>
#include <functional>
#include <string>

class IO
{
public:
    virtual ~IO();

    virtual size_t size() const = 0;
    virtual size_t tell() const = 0;
    virtual void seek(size_t offset) = 0;
    virtual void skip(int offset) = 0;
    virtual void truncate(size_t new_size) = 0;
    void peek(size_t offset, std::function<void()> func);
    bool eof() const;

    virtual void read(void *input, size_t length) = 0;
    virtual void write(const void *str, size_t length) = 0;
    virtual void write_from_io(IO &input, size_t length) = 0;
    void write_from_io(IO &input);

    void read_until_zero(char **output, size_t *output_size);
    std::string read_until_zero();
    std::string read_until_zero(size_t bytes);
    std::string read_until_end();
    std::string read(size_t bytes);
    uint8_t read_u8();
    uint16_t read_u16_le();
    uint16_t read_u16_be();
    uint32_t read_u32_le();
    uint32_t read_u32_be();
    uint64_t read_u64_le();
    uint64_t read_u64_be();

    void write(const std::string &bytes);
    void write_u8(uint8_t);
    void write_u16_le(uint16_t);
    void write_u16_be(uint16_t);
    void write_u32_le(uint32_t);
    void write_u32_be(uint32_t);
    void write_u64_le(uint64_t);
    void write_u64_be(uint64_t);
};

#endif
