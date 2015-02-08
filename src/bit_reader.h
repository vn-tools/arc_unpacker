#ifndef BIT_READER_H
#define BIT_READER_H
#include "buffered_io.h"

class BitReader
{
public:
    BitReader(BufferedIO &io);
    ~BitReader();
    bool get();
    unsigned int get(size_t n);
private:
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
