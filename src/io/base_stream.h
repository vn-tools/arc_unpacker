#pragma once

#include "io/istream.h"

namespace au {
namespace io {

    class BaseStream : public IStream
    {
    public:
        virtual ~BaseStream() {}

        IStream &peek(
            const size_t offset, const std::function<void()> func) override;
        bool eof() const override;

        bstr read_to_zero() override;
        bstr read_to_zero(const size_t bytes) override;
        bstr read_to_eof() override;
        bstr read_line() override;
    };

} }
