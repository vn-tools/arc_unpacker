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

#include "dec/kaguya/bmr_file_decoder.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

static const io::path dir = "tests/dec/kaguya/files/bmr/";

TEST_CASE("Atelier Kaguya BMR files", "[dec]")
{
    const auto bmr_decoder = BmrFileDecoder();
    const auto input_file = tests::file_from_path(dir / "bg_black.bmp");
    const auto expected_file = tests::file_from_path(dir / "bg_black-out.png");
    const auto actual_file = tests::decode(bmr_decoder, *input_file);
    const auto bmp_decoder = dec::microsoft::BmpImageDecoder();
    const auto actual_image = tests::decode(bmp_decoder, *actual_file);
    tests::compare_images(actual_image, *expected_file);
}
