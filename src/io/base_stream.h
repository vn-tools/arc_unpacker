#pragma once

#include <functional>
#include <memory>

namespace au {
namespace io {

    class BaseStream
    {
    public:
        virtual ~BaseStream() = 0;

        bool eof() const;
        size_t left() const;
        virtual size_t size() const = 0;
        virtual size_t tell() const = 0;
        virtual BaseStream &seek(const size_t offset) = 0;
        virtual BaseStream &resize(const size_t new_size) = 0;

        // virtual allows changing return type in method chaining
        virtual BaseStream &skip(const int offset);
        virtual BaseStream &peek(
            const size_t offset, const std::function<void()> func);
    };

} }
