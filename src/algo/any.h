// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <type_traits>
#include <typeinfo>
#include "algo/format.h"
#include "err.h"

namespace au {
namespace algo {

    class Any final
    {
    public:
        Any() : ptr(nullptr) {}
        Any(Any &&other) : ptr(std::move(other.ptr)) {}
        Any(const Any &other) : ptr(other.ptr ? other.ptr->clone() : nullptr) {}

        template<typename T> Any(const T &value)
            : ptr(new Holder<typename std::decay<const T>::type>(value)) {}
        template<typename T> Any(T &&value)
            : ptr(new Holder<typename std::decay<T>::type>(
                static_cast<T&&>(value))) {}

        ~Any() {}

        Any &swap(Any &other)
        {
            std::swap(ptr, other.ptr);
            return *this;
        }

        Any &operator=(const Any &other)
        {
            Any(other).swap(*this);
            return *this;
        }

        Any &operator=(Any &&other)
        {
            other.swap(*this);
            Any().swap(other);
            return *this;
        }

        template<typename T> Any &operator=(T &&other)
        {
            Any(static_cast<T&&>(other)).swap(*this);
            return *this;
        }

        template<typename T> T get() const
        {
            if (empty())
                throw err::GeneralError("Dereferencing nullptr");
            if (ptr->type() != typeid(T))
            {
                throw err::GeneralError(algo::format(
                    "Type '%s' does not match type '%s'",
                    ptr->type().name(),
                    typeid(T).name()));
            }
            return static_cast<Holder<T>*>(ptr.get())->value;
        }

        void clear() { ptr.reset(nullptr); }
        bool empty() const { return ptr == nullptr; }
        operator bool() const { return !empty(); }

        const std::type_info &type() const
        {
            return empty() ? typeid(nullptr) : ptr->type();
        }

    private:
        struct IPlaceholder
        {
            virtual ~IPlaceholder() {}
            virtual const std::type_info &type() const = 0;
            virtual std::unique_ptr<IPlaceholder> clone() const = 0;
        };

        template<typename T> struct Holder final : public IPlaceholder
        {
            Holder(T &&value) : value(static_cast<T&&>(value)) {}
            Holder(const T &value) : value(value) {}

            std::unique_ptr<IPlaceholder> clone() const override
            {
                return std::make_unique<Holder<T>>(value);
            }

            const std::type_info &type() const override
            {
                return typeid(T);
            }

            Holder &operator=(const Holder &);

            T value;
        };

        std::unique_ptr<IPlaceholder> ptr;
    };

    using any = Any;

} }
