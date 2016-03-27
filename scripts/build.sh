#!/bin/bash
set -ev
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    cd windows
    make  GCC=i586-mingw32msvc-gcc WINDRES=i586-mingw32msvc-windres
fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    cd mac
    make
fi