#pragma once

#include <functional>
#include "types.h"

namespace au {
namespace fmt {

    class INamingStrategy
    {
    public:
        virtual std::string decorate(
            const std::string &parent_name,
            const std::string &current_name) const = 0;
    };

    class RootNamingStrategy final : public INamingStrategy
    {
        virtual std::string decorate(
            const std::string &parent_name,
            const std::string &current_name) const override;
    };

    class SiblingNamingStrategy final : public INamingStrategy
    {
        virtual std::string decorate(
            const std::string &parent_name,
            const std::string &current_name) const override;
    };

    class ChildNamingStrategy final : public INamingStrategy
    {
        virtual std::string decorate(
            const std::string &parent_name,
            const std::string &current_name) const override;
    };

} }
