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

#include <array>
#include <functional>
#include <memory>
#include "types.h"

namespace au {
namespace algo {
namespace crypt {

    class MersenneTwister final
    {
    public:
        static std::unique_ptr<MersenneTwister> Knuth(const u32 seed);
        static std::unique_ptr<MersenneTwister> Classic(const u32 seed);
        static std::unique_ptr<MersenneTwister> Improved(const u32 seed);

        ~MersenneTwister();

        u32 next_u32();

    private:
        struct Priv;
        MersenneTwister(
            const std::function<void(Priv*, const u32)> seed_func,
            const u32 default_seed);
        std::unique_ptr<Priv> p;
    };

} } }
