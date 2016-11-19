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

#include "dec/borland/tpf0_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::borland;

static const std::string dir = "tests/dec/borland/files/tpf0/";

TEST_CASE("Borland TPF0 decoder", "[dec]")
{
    using namespace std;

    SECTION("Test 1")
    {
        const auto input_file = tests::file_from_path(dir + "test1.dat");
        const auto input_data = input_file->stream.seek(0).read_to_eof();
        const auto decoder = Tpf0Decoder();
        const auto r = decoder.decode(input_data);

        REQUIRE(r != nullptr);
        REQUIRE(r->type == "TFSAF");
        REQUIRE(r->name == "FSAF");
        REQUIRE(r->properties.size() == 8);
        REQUIRE(r->property<int>("Left") == 538);
        REQUIRE(r->property<int>("Top") == 359);
        REQUIRE(r->property<int>("Width") == 400);
        REQUIRE(r->property<int>("Height") == 170);
        REQUIRE(r->property<int>("ClientWidth") == 400);
        REQUIRE(r->property<int>("ClientHeight") == 170);
        REQUIRE(r->property<string>("Caption") == "FSAF");
        REQUIRE(r->property<string>("LCLVersion") == "0.9.28.2");
        REQUIRE(r->children.size() == 14);

        REQUIRE(r->children[0]->type == "TLabel");
        REQUIRE(r->children[0]->name == "L_detailSAF");
        REQUIRE(r->children[0]->properties.size() == 6);
        REQUIRE(r->children[0]->property<int>("Left") == 2);
        REQUIRE(r->children[0]->property<int>("Top") == 1);
        REQUIRE(r->children[0]->property<int>("Width") == 61);
        REQUIRE(r->children[0]->property<int>("Height") == 16);
        REQUIRE(r->children[0]->property<string>("Caption") == "L_detailSAF");
        REQUIRE(r->children[0]->property<bool>("ParentColor") == true);
        REQUIRE(r->children[0]->children.empty());

        REQUIRE(r->children[1]->type == "TLabel");
        REQUIRE(r->children[1]->name == "L_nomSAD0");
        REQUIRE(r->children[1]->properties.size() == 7);
        REQUIRE(r->children[1]->children.empty());

        REQUIRE(r->children[2]->type == "TLabel");
        REQUIRE(r->children[2]->name == "L_nomSAD1");
        REQUIRE(r->children[2]->properties.size() == 7);
        REQUIRE(r->children[2]->children.empty());

        REQUIRE(r->children[3]->type == "TLabel");
        REQUIRE(r->children[3]->name == "L_nomSAD2");
        REQUIRE(r->children[3]->properties.size() == 7);
        REQUIRE(r->children[3]->children.empty());

        REQUIRE(r->children[4]->type == "TLabel");
        REQUIRE(r->children[4]->name == "L_nomSAD3");
        REQUIRE(r->children[4]->properties.size() == 6);
        REQUIRE(r->children[4]->children.empty());

        REQUIRE(r->children[5]->type == "TBitBtn");
        REQUIRE(r->children[5]->name == "BBok");
        REQUIRE(r->children[5]->properties.size() == 10);
        REQUIRE(r->children[5]->children.empty());

        REQUIRE(r->children[6]->type == "TBitBtn");
        REQUIRE(r->children[6]->name == "BBhelp");
        REQUIRE(r->children[6]->properties.size() == 8);
        REQUIRE(r->children[6]->children.empty());

        REQUIRE(r->children[7]->type == "TBitBtn");
        REQUIRE(r->children[7]->name == "BBabandon");
        REQUIRE(r->children[7]->properties.size() == 9);
        REQUIRE(r->children[7]->children.empty());

        REQUIRE(r->children[8]->type == "TButton");
        REQUIRE(r->children[8]->name == "B_choixpiloteSAF");
        REQUIRE(r->children[8]->properties.size() == 7);
        REQUIRE(r->children[8]->children.empty());

        REQUIRE(r->children[9]->type == "TFloatSpinEdit");
        REQUIRE(r->children[9]->name == "FSE_SAD0");
        REQUIRE(r->children[9]->properties.size() == 11);
        REQUIRE(r->children[9]->property<f64>("MaxValue") == 3.953125);
        REQUIRE(r->children[9]->children.empty());

        REQUIRE(r->children[10]->type == "TFloatSpinEdit");
        REQUIRE(r->children[10]->name == "FSE_SAD1");
        REQUIRE(r->children[10]->properties.size() == 11);
        REQUIRE(r->children[10]->children.empty());

        REQUIRE(r->children[11]->type == "TFloatSpinEdit");
        REQUIRE(r->children[11]->name == "FSE_SAD2");
        REQUIRE(r->children[11]->properties.size() == 11);
        REQUIRE(r->children[11]->children.empty());

        REQUIRE(r->children[12]->type == "TFloatSpinEdit");
        REQUIRE(r->children[12]->name == "FSE_SAD3");
        REQUIRE(r->children[12]->properties.size() == 9);
        REQUIRE(r->children[12]->children.empty());

        REQUIRE(r->children[13]->type == "TOpenDialog");
        REQUIRE(r->children[13]->name == "OD_choixpilsaf");
        REQUIRE(r->children[13]->properties.size() == 3);
        REQUIRE(r->children[13]->children.empty());
    }

    SECTION("Test 2")
    {
        const auto input_file
            = tests::zlib_file_from_path(dir + "test2-zlib.dat");
        const auto input_data = input_file->stream.seek(0).read_to_eof();
        const auto decoder = Tpf0Decoder();
        const auto r = decoder.decode(input_data);

        REQUIRE(r != nullptr);
        REQUIRE(r->type == "TForm1");
        REQUIRE(r->name == "Form1");
        REQUIRE(r->properties.size() == 23);
        REQUIRE(r->property<int>("Left") == 381);
        REQUIRE(r->property<int>("Top") == 236);
        REQUIRE(r->property<int>("ClientWidth") == 317);
        REQUIRE(r->property<int>("ClientHeight") == 238);
        REQUIRE(r->property<string>("Color") == "clBtnFace");
        REQUIRE(r->property<int>("PixelsPerInch") == 96);
        REQUIRE(r->property<int>("TextHeight") == 12);
        REQUIRE(r->property<int>("Font.Height") == 244);
        REQUIRE(r->property<string>("Font.Name") == "MS ????");
        REQUIRE(r->property<string>("Font.Charset") == "SHIFTJIS_CHARSET");
        REQUIRE(r->property<string>("Font.Color") == "clWindowText");
        REQUIRE(r->property<vector<string>>("Font.Style").empty());
        REQUIRE(r->property<string>("OnCanResize") == "FormCanResize");
        REQUIRE(r->property<string>("OnClose") == "FormClose");
        REQUIRE(r->property<string>("OnCreate") == "FormCreate");
        REQUIRE(r->property<string>("OnDestroy") == "FormDestroy");
        REQUIRE(r->property<string>("OnKeyDown") == "FormKeyDown");
        REQUIRE(r->property<string>("OnKeyUp") == "FormKeyUp");
        REQUIRE(r->property<string>("OnPaint") == "FormPaint");
        REQUIRE(r->property<string>("OnResize") == "FormResize");
        REQUIRE(r->property<vector<string>>("BorderIcons").size() == 2);
        REQUIRE(r->property<vector<string>>("BorderIcons")[0]
            == "biSystemMenu");
        REQUIRE(r->property<vector<string>>("BorderIcons")[1] == "biMinimize");
        REQUIRE(r->children.size() == 3);

        REQUIRE(r->children[0]->type == "TImage");
        REQUIRE(r->children[0]->name == "IconKeyImage");
        REQUIRE(r->children[0]->properties.size() == 6);
        REQUIRE(r->children[0]->property<int>("Left") == 8);
        REQUIRE(r->children[0]->property<int>("Top") == 8);
        REQUIRE(r->children[0]->property<int>("Width") == 105);
        REQUIRE(r->children[0]->property<int>("Height") == 105);
        REQUIRE(r->children[0]->property<bool>("Visible") == true);
        REQUIRE(r->children[0]->property<bstr>("Picture.Data").substr(0, 7)
            == "\x05TIcon\x00"_b);
        REQUIRE(r->children[0]->children.empty());

        REQUIRE(r->children[1]->type == "TDDSD7");
        REQUIRE(r->children[1]->name == "DDSD71");
        REQUIRE(r->children[1]->properties.size() == 4);
        REQUIRE(r->children[1]->property<int>("ChannelCount") == 16);
        REQUIRE(r->children[1]->property<int>("Left") == 244);
        REQUIRE(r->children[1]->property<int>("Top") == 4);
        REQUIRE(r->children[1]->property<vector<string>>("DebugOption").size()
            == 1);
        REQUIRE(r->children[1]->property<vector<string>>("DebugOption")[0]
            == "dsoHaltOnError");
        REQUIRE(r->children[1]->children.empty());

        REQUIRE(r->children[2]->type == "TPopupMenu");
        REQUIRE(r->children[2]->name == "PopupMenu1");
        REQUIRE(r->children[2]->properties.size() == 2);
        REQUIRE(r->children[2]->property<int>("Left") == 244);
        REQUIRE(r->children[2]->property<int>("Top") == 40);
        REQUIRE(r->children[2]->children.size() == 1);

        REQUIRE(r->children[2]->children[0]->type == "TMenuItem");
        REQUIRE(r->children[2]->children[0]->name == "Show1");
        REQUIRE(r->children[2]->children[0]->properties.size() == 2);
        REQUIRE(r->children[2]->children[0]->property<string>("Caption")
            == u8"表示");
        REQUIRE(r->children[2]->children[0]->property<string>("OnClick")
            == "Show1Click");
        REQUIRE(r->children[2]->children[0]->children.empty());
    }
}
