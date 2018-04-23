#! /bin/bash

g++ --version
cmake --version

if [ "$QTTYPE" = "5" ]; then
    ./configure --tests --extension
    cd release
    make || exit 1
fi
