#pragma once

#include <algorithm>
#include <climits>
#include <stdexcept>
#include "types.h"

#ifndef __BYTE_ORDER__
    #if defined(_WIN32)
        #include <winsock2.h>

        #if BYTE_ORDER == LITTLE_ENDIAN
            #define AU_UTIL_ENDIAN_LITTLE_ENDIAN
        #elif BYTE_ORDER == BIG_ENDIAN
            #define AU_UTIL_ENDIAN_BIG_ENDIAN
        #else
            #error "unsupported endianness"
        #endif
    #else
        #error "byte order not defined"
    #endif
#else
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define AU_UTIL_ENDIAN_LITTLE_ENDIAN
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define AU_UTIL_ENDIAN_BIG_ENDIAN
    #else
        #error "unsupported endianness"
    #endif
#endif

namespace au {
namespace algo {

    enum class Endianness : u8
    {
        LittleEndian,
        BigEndian,
    };

    namespace priv {

        template<typename T> inline T swap_bytes(const T val)
        {
            T ret = val;
            char *x = reinterpret_cast<char*>(&ret);
            std::reverse(x, x + sizeof(T));
            return ret;
        }

    }

    constexpr Endianness get_machine_endianness()
    {
        #if defined(AU_UTIL_ENDIAN_LITTLE_ENDIAN)
            return Endianness::LittleEndian;
        #elif defined(AU_UTIL_ENDIAN_BIG_ENDIAN)
            return Endianness::BigEndian;
        #endif
    }

    template<typename T> inline T from_little_endian(const T value)
    {
        return get_machine_endianness() != Endianness::LittleEndian
            ? priv::swap_bytes<T>(value)
            : value;
    }

    template<typename T> inline T from_big_endian(const T value)
    {
        return get_machine_endianness() != Endianness::BigEndian
            ? priv::swap_bytes<T>(value)
            : value;
    }

    template<typename T> inline T to_little_endian(const T value)
    {
        return from_little_endian(value);
    }

    template<typename T> inline T to_big_endian(const T value)
    {
        return from_big_endian(value);
    }

} }
