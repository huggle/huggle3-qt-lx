#! /bin/bash

if [ "$QTTYPE" = "5" ]; then
    ./configure --tests --extension --web-engine --qtpath /usr/local/opt/qt/
    cd release
    make || exit 1
fi
