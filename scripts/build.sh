#!/bin/bash
set -ev
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    sudo apt update
    sudo apt install g++-mingw-w64
    cd windows
    make GXX=x86_64-w64-mingw32-g++ WINDRES=x86_64-w64-mingw32-windres

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
