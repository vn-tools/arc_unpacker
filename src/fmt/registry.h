#pragma once

#include <vector>
#include "transformer.h"

namespace au {
namespace fmt {

    class Transformer;

    class Registry final
    {
    private:
        using TransformerCreator
            = std::function<std::unique_ptr<Transformer>()>;

    public:
        static Registry &instance();
        const std::vector<std::string> get_names() const;
        std::unique_ptr<Transformer> create(const std::string &name) const;

        template<typename T> static bool add(const std::string &name)
        {
            Registry::instance().add([]()
            {
                return std::unique_ptr<Transformer>(new T());
            }, name);
            return true;
        }

    private:
        Registry();
        ~Registry();

        void add(TransformerCreator creator, const std::string &name);

        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
