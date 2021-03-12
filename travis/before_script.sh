#! /bin/bash

g++ --version
cmake --version

if [ "$QTTYPE" = "5" ]; then
    ./configure --tests --extension --web-engine
    cd release
    make || exit 1
fi
