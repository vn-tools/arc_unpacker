#include "res/image.h"
#include "algo/range.h"
#include "test_support/catch.h"

using namespace au;

static res::Image create_overlay(const size_t width, const size_t height)
{
    res::Image overlay(width, height);
    for (const auto x : algo::range(overlay.width()))
    for (const auto y : algo::range(overlay.height()))
    {
        overlay.at(x, y).r = x;
        overlay.at(x, y).g = y;
    }
    return overlay;
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
        const auto overlay = create_overlay(5, 5);
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
        const auto overlay = create_overlay(5, 5);
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
        const auto overlay = create_overlay(5, 5);
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
        const auto overlay = create_overlay(100, 100);
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
        const auto overlay = create_overlay(3, 3);
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
        const auto overlay = create_overlay(5, 5);
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
        const auto overlay = create_overlay(5, 5);
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
