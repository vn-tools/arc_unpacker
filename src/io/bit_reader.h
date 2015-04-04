#ifndef IO_BIT_READER_H
#define IO_BIT_READER_H
#include <memory>
#include "io/io.h"

class BitReader
{
public:
    BitReader(IO &io);
    BitReader(const char *buffer, size_t buffer_size);
    ~BitReader();

    unsigned int get(size_t n);
    unsigned int try_get(size_t n);
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
