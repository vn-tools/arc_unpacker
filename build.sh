#!/bin/bash

dirs=lib/ivory

for x in $dirs; do
    pushd $x
    ruby extconfig.rb && make
    popd
done
