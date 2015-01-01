#!/bin/bash

dirs=lib/formats/gfx/prs_converter

for x in $dirs; do
    pushd $x
    ruby extconfig.rb && make
    popd
done
