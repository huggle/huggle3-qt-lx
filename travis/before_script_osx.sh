#! /bin/bash

ls /usr/local/Cellar/qt/*
if [ "$QTTYPE" = "5" ]; then
    ./configure --tests --extension --web-engine --qtpath /usr/local/Cellar/qt/*/clang_64
    cd release
    make || exit 1
fi
