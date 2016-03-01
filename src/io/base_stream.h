#pragma once

#include <functional>
#include <memory>
#include "types.h"

namespace au {
namespace io {

    class BaseStream
    {
    public:
        virtual ~BaseStream() = 0;

        uoff_t left() const;
        virtual uoff_t size() const = 0;
        virtual uoff_t pos() const = 0;
        virtual BaseStream &seek(const uoff_t offset) = 0;
        virtual BaseStream &resize(const uoff_t new_size) = 0;

        // virtual allows changing return type in method chaining
        virtual BaseStream &skip(const soff_t offset);
        virtual BaseStream &peek(
            const uoff_t offset, const std::function<void()> &func);
    };

} }
