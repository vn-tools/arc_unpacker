#!/bin/bash

dirs=( lib/formats/gfx/prs_converter lib/formats/gfx/spb_converter lib/formats/gfx/g00_converter lib/formats/gfx/tlg_converter lib/formats/gfx/cbg_converter )

for x in "${dirs[@]}"; do
    pushd "$x"
    ruby extconfig.rb && make || exit 1
    popd
done
