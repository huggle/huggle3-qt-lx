#! /bin/bash

# this qtpath is for Qt5 only
#./configure --qt6 --tests --extension --web-engine --qtpath /usr/local/opt/qt/
./configure --qt6 --tests --extension --web-engine --qtpath /Users/runner/work/huggle3-qt-lx/Qt/6.5.3/macos/
cd release
make || exit 1
