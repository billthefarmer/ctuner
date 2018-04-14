#!/bin/bash
set -ev
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    cd windows
    make GPLUSPLUS=i586-mingw32msvc-g++ WINDRES=i586-mingw32msvc-windres

#    cd ../linux
#    ./configure
#    make
fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    cd mac
    make
    cd ../swift
    make
fi
