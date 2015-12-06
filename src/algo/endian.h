#pragma once

#include <climits>
#include <cstdint>
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

    namespace priv {

        template<typename T, size_t sz> struct swap_bytes final
        {
            T operator()(const T val)
            {
                throw std::logic_error("Unsupported data size");
            }
        };

        template<typename T> struct swap_bytes<T, 1> final
        {
            constexpr T operator()(const T val)
            {
                return val;
            }
        };

        template<typename T> struct swap_bytes<T, 2> final
        {
            constexpr T operator()(const T val)
            {
                return __builtin_bswap16(val);
            }
        };

        template<typename T> struct swap_bytes<T, 4> final
        {
            constexpr T operator()(const T val)
            {
                return __builtin_bswap32(val);
            }
        };

        template<typename T> struct swap_bytes<T, 8> final
        {
            constexpr T operator()(const T val)
            {
                return __builtin_bswap64(val);
            }
        };

    }

    template<typename T> constexpr T from_little_endian(const T value)
    {
        #if defined(AU_UTIL_ENDIAN_LITTLE_ENDIAN)
            return value;
        #elif defined(AU_UTIL_ENDIAN_BIG_ENDIAN)
            return priv::swap_bytes<T, sizeof(T)>()(value);
        #endif
    }

    template<typename T> constexpr T from_big_endian(const T value)
    {
        #if defined(AU_UTIL_ENDIAN_BIG_ENDIAN)
            return value;
        #elif defined(AU_UTIL_ENDIAN_LITTLE_ENDIAN)
            return priv::swap_bytes<T, sizeof(T)>()(value);
        #endif
    }

    template<typename T> constexpr T to_little_endian(const T value)
    {
        return from_little_endian(value);
    }

    template<typename T> constexpr T to_big_endian(const T value)
    {
        return from_big_endian(value);
    }

} }
