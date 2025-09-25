#! /bin/bash

./configure --qt6 --tests --extension --web-engine --qtpath /usr/local/opt/qt/
cd release
make || exit 1
