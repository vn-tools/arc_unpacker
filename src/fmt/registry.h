#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace au {
namespace fmt {

    class IDecoder;

    class Registry final
    {
    private:
        using DecoderCreator
            = std::function<std::unique_ptr<IDecoder>()>;

    public:
        ~Registry();
        static Registry &instance();
        static std::unique_ptr<Registry> create_mock();

        const std::vector<std::string> get_decoder_names() const;
        bool has_decoder(const std::string &name) const;
        void add_decoder(const std::string &name, DecoderCreator creator);
        std::unique_ptr<IDecoder> create_decoder(const std::string &name) const;

    private:
        Registry();

        struct Priv;
        std::unique_ptr<Priv> p;
    };

    template<typename T> static bool register_fmt(const std::string &name)
    {
        Registry::instance().add_decoder(
            name, []() { return std::unique_ptr<IDecoder>(new T()); });
        return true;
    }

} }
