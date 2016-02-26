#pragma once

#include "algo/crypt/camellia.h"
#include "io/base_byte_stream.h"

namespace au {
namespace dec {
namespace malie {
namespace common {

    // Rather than decrypting to bstr, the decryption is implemented as stream,
    // so that huge files occupy as little memory as possible
    class CamelliaStream final : public io::BaseByteStream
    {
    public:
        CamelliaStream(
            io::BaseByteStream &parent_stream, const std::vector<u32> &key);
        CamelliaStream(
            io::BaseByteStream &parent_stream,
            const std::vector<u32> &key,
            const size_t offset,
            const size_t size);
        ~CamelliaStream();

        size_t size() const override;
        size_t pos() const override;
        std::unique_ptr<BaseByteStream> clone() const override;

    protected:
        void read_impl(void *destination, const size_t size) override;
        void write_impl(const void *source, const size_t size) override;
        void seek_impl(const size_t offset) override;
        void resize_impl(const size_t new_size) override;

    private:
        const std::vector<u32> key;
        std::unique_ptr<algo::crypt::Camellia> camellia;
        std::unique_ptr<io::BaseByteStream> parent_stream;
        const size_t parent_stream_offset;
        const size_t parent_stream_size;
    };

} } } }
