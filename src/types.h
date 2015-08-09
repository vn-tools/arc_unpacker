#ifndef AU_TYPES_H
#define AU_TYPES_H
#include <cstdint>
#include <cstddef>
#include <string>
#include "bstr.h"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

constexpr u8 operator "" _u8(char value)
{
    return static_cast<u8>(value);
}

constexpr const u8* operator "" _u8(const char *value, size_t n)
{
    return reinterpret_cast<const u8*>(value);
}

inline bstr operator "" _b(const char *value, size_t n)
{
    return bstr(value, n);
}

#endif
