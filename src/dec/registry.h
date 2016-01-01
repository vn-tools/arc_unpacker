#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace au {
namespace dec {

    class IDecoder;

    class Registry final
    {
    private:
        using DecoderCreator
            = std::function<std::shared_ptr<IDecoder>()>;

    public:
        ~Registry();
        static Registry &instance();
        static std::unique_ptr<Registry> create_mock();

        const std::vector<std::string> get_decoder_names() const;
        bool has_decoder(const std::string &name) const;
        void add_decoder(const std::string &name, DecoderCreator creator);
        std::shared_ptr<IDecoder> create_decoder(const std::string &name) const;

    private:
        Registry();

        struct Priv;
        std::unique_ptr<Priv> p;
    };

    template <typename T, typename ...Params> bool register_decoder(
        const std::string &name, Params&&... params)
    {
        Registry::instance().add_decoder(
            name, [=]() { return std::make_shared<T>(params...); });
        return true;
    }

} }
