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

#include "res/image.h"
#include "algo/range.h"
#include "test_support/catch.h"

using namespace au;

static res::Image create_test_image(const size_t width, const size_t height)
{
    res::Image test_image(width, height);
    for (const auto x : algo::range(test_image.width()))
    for (const auto y : algo::range(test_image.height()))
    {
        test_image.at(x, y).r = x;
        test_image.at(x, y).g = y;
    }
    return test_image;
}

TEST_CASE("Image overlays", "[res]")
{
    // I - intersection
    // B - base image
    // O - overlay image

    // +---+
    // |III|
    // |III|
    // +---+
    SECTION("Simple 1:1 overlay")
    {
        const auto overlay = create_test_image(5, 5);
        res::Image base(5, 5);
        base.overlay(overlay, res::Image::OverlayKind::OverwriteAll);
        for (const auto x : algo::range(base.width()))
        for (const auto y : algo::range(base.height()))
        {
            REQUIRE(base.at(x, y).r == x);
            REQUIRE(base.at(x, y).g == y);
        }
    }

    // +---+
    // |B+-+-+
    // |B|I|O|
    // +-+-+O|
    //   +---+
    SECTION("Overlay at positive offsets")
    {
        const auto overlay = create_test_image(5, 5);
        res::Image base(5, 5);
        base.overlay(overlay, 2, 2, res::Image::OverlayKind::OverwriteAll);
        for (const auto x : algo::range(2, base.width()))
        for (const auto y : algo::range(2, base.height()))
        {
            REQUIRE(base.at(x, y).r == x - 2);
            REQUIRE(base.at(x, y).g == y - 2);
        }
    }

    // +---+
    // |O+-+-+
    // |O|I|B|
    // +-+-+B|
    //   +---+
    SECTION("Overlay at negative offsets")
    {
        const auto overlay = create_test_image(5, 5);
        res::Image base(5, 5);
        base.overlay(overlay, -2, -1, res::Image::OverlayKind::OverwriteAll);
        for (const auto x : algo::range(3))
        for (const auto y : algo::range(4))
        {
            REQUIRE(base.at(x, y).r == x + 2);
            REQUIRE(base.at(x, y).g == y + 1);
        }
    }

    // +-------+
    // |O+---+O|
    // |O|III|O|
    // |O|III|O|
    // |O+---+O|
    // +-------|
    SECTION("Overlay fully covering base image")
    {
        const auto overlay = create_test_image(100, 100);
        res::Image base(5, 5);
        base.overlay(overlay, -50, -50, res::Image::OverlayKind::OverwriteAll);
        for (const auto x : algo::range(base.width()))
        for (const auto y : algo::range(base.height()))
        {
            REQUIRE(base.at(x, y).r == x + 50);
            REQUIRE(base.at(x, y).g == y + 50);
        }
    }

    // +-------+
    // |B+---+B|
    // |B|III|B|
    // |B|III|B|
    // |B+---+B|
    // +-------|
    SECTION("Overlay fully contained in base image")
    {
        const auto overlay = create_test_image(3, 3);
        res::Image base(5, 5);
        base.overlay(overlay, 1, 1, res::Image::OverlayKind::OverwriteAll);
        for (const auto x : algo::range(1, 3))
        for (const auto y : algo::range(1, 3))
        {
            REQUIRE(base.at(x, y).r == x - 1);
            REQUIRE(base.at(x, y).g == y - 1);
        }
    }

    // +---+
    // |BBB| +---+
    // |BBB| |OOO|
    // +---+ |OOO|
    //       +---+
    SECTION("Overlay completely hidden at positive offsets")
    {
        const auto overlay = create_test_image(5, 5);
        res::Image base(5, 5);
        base.overlay(
            overlay, 1000, 1000, res::Image::OverlayKind::OverwriteAll);
        for (const auto x : algo::range(base.width()))
        for (const auto y : algo::range(base.height()))
        {
            REQUIRE(base.at(x, y).r == 0);
            REQUIRE(base.at(x, y).g == 0);
        }
    }

    // +---+
    // |OOO| +---+
    // |OOO| |BBB|
    // +---+ |BBB|
    //       +---+
    SECTION("Overlay completely hidden at negative offsets")
    {
        const auto overlay = create_test_image(5, 5);
        res::Image base(5, 5);
        base.overlay(
            overlay, -1000, -1000, res::Image::OverlayKind::OverwriteAll);
        for (const auto x : algo::range(base.width()))
        for (const auto y : algo::range(base.height()))
        {
            REQUIRE(base.at(x, y).r == 0);
            REQUIRE(base.at(x, y).g == 0);
        }
    }
}

TEST_CASE("Image cropping", "[res]")
{
    SECTION("Cutting pixels")
    {
        auto image = create_test_image(5, 5);
        image.crop(3, 4);
        REQUIRE(image.width() == 3);
        REQUIRE(image.height() == 4);
        for (const auto x : algo::range(image.width()))
        for (const auto y : algo::range(image.height()))
        {
            REQUIRE(image.at(x, y).r == x);
            REQUIRE(image.at(x, y).g == y);
        }
    }

    SECTION("Expanding pixels")
    {
        const auto orig_width = 5;
        const auto orig_height = 5;
        auto image = create_test_image(orig_width, orig_height);
        image.crop(6, 7);
        REQUIRE(image.width() == 6);
        REQUIRE(image.height() == 7);
        for (const auto x : algo::range(5))
        for (const auto y : algo::range(5))
        {
            REQUIRE(image.at(x, y).r == x);
            REQUIRE(image.at(x, y).g == y);
            REQUIRE(image.at(x, y).a == 0);
        }
        for (const auto x : algo::range(orig_width, image.width()))
        for (const auto y : algo::range(image.height()))
        {
            REQUIRE(image.at(x, y).r == 0);
            REQUIRE(image.at(x, y).g == 0);
            REQUIRE(image.at(x, y).a == 0);
        }
        for (const auto x : algo::range(image.width()))
        for (const auto y : algo::range(orig_width, image.height()))
        {
            REQUIRE(image.at(x, y).r == 0);
            REQUIRE(image.at(x, y).g == 0);
            REQUIRE(image.at(x, y).a == 0);
        }
    }
}

TEST_CASE("Image offsetting", "[res]")
{
    SECTION("Cutting pixels")
    {
        const auto offset_x = 4;
        const auto offset_y = 3;
        const auto orig_width = 5;
        const auto orig_height = 5;
        auto image = create_test_image(orig_width, orig_height);
        image.offset(-offset_x, -offset_y);
        REQUIRE(image.width() == orig_width - offset_x);
        REQUIRE(image.height() == orig_height - offset_y);
        for (const auto x : algo::range(image.width()))
        for (const auto y : algo::range(image.height()))
        {
            REQUIRE(image.at(x, y).r == x + offset_x);
            REQUIRE(image.at(x, y).g == y + offset_y);
        }
    }

    SECTION("Expanding pixels")
    {
        const auto offset_x = 4;
        const auto offset_y = 3;
        const auto orig_width = 5;
        const auto orig_height = 5;
        auto image = create_test_image(orig_width, orig_height);
        image.offset(offset_x, offset_y);
        REQUIRE(image.width() == orig_width + offset_x);
        REQUIRE(image.height() == orig_height + offset_y);
        for (const auto x : algo::range(offset_x, image.width()))
        for (const auto y : algo::range(offset_y, image.height()))
        {
            REQUIRE(image.at(x, y).r == x - offset_x);
            REQUIRE(image.at(x, y).g == y - offset_y);
        }
        for (const auto x : algo::range(offset_x))
        for (const auto y : algo::range(image.height()))
        {
            REQUIRE(image.at(x, y).r == 0);
            REQUIRE(image.at(x, y).g == 0);
            REQUIRE(image.at(x, y).a == 0);
        }
        for (const auto x : algo::range(image.width()))
        for (const auto y : algo::range(offset_y))
        {
            REQUIRE(image.at(x, y).r == 0);
            REQUIRE(image.at(x, y).g == 0);
            REQUIRE(image.at(x, y).a == 0);
        }
    }
}
