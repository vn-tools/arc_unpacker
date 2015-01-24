#!/bin/bash

dirs=()
dirs+=(lib/formats/gfx/prs_converter)
dirs+=(lib/formats/gfx/spb_converter)
dirs+=(lib/formats/gfx/g00_converter)
dirs+=(lib/formats/gfx/tlg_converter)
dirs+=(lib/formats/gfx/cbg_converter)

for x in "${dirs[@]}"; do
    pushd "$x"
    ruby extconfig.rb && make || exit 1
    popd
done
