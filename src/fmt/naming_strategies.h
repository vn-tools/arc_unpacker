#pragma once

#include <functional>
#include "io/path.h"
#include "types.h"

namespace au {
namespace fmt {

    class INamingStrategy
    {
    public:
        virtual io::path decorate(
            const io::path &parent_name,
            const io::path &current_name) const = 0;
    };

    class RootNamingStrategy final : public INamingStrategy
    {
        io::path decorate(
            const io::path &parent_name,
            const io::path &current_name) const override;
    };

    class SiblingNamingStrategy final : public INamingStrategy
    {
        io::path decorate(
            const io::path &parent_name,
            const io::path &current_name) const override;
    };

    class ChildNamingStrategy final : public INamingStrategy
    {
        io::path decorate(
            const io::path &parent_name,
            const io::path &current_name) const override;
    };

} }
