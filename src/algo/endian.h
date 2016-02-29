#pragma once

#include <algorithm>
#include <stdexcept>
#include "types.h"

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
        // this is verified in the tests
        return Endianness::LittleEndian;
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
