#pragma once

#include <map>
#include "algo/any.h"
#include "types.h"

namespace au {
namespace dec {
namespace borland {

    struct Tpf0Structure final
    {
        std::string type;
        std::string name;
        std::map<std::string, algo::any> properties;
        std::vector<std::unique_ptr<Tpf0Structure>> children;

        template<typename T> const T property(const std::string &key) const
        {
            return properties.at(key).get<T>();
        }
    };

    class Tpf0Decoder final
    {
    public:
        std::unique_ptr<Tpf0Structure> decode(const bstr &input) const;
    };

} } }
