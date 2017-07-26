#! /bin/bash

cd ./huggle

if [ "$QTTYPE" = "4" ]; then
	./configure --qt4 --extension --no-audio
    cd huggle_release
	make || exit 1
    cd ..
	cd tests/test
	cmake .
	make || exit 1
fi

if [ "$QTTYPE" = "5" ]; then
    ./configure --qt5 --extension --web-engine --qtpath /usr/local/opt/qt/
    cd huggle_release
    make || exit 1
    cd ..
	cd tests/test
    cmake . -DCMAKE_PREFIX_PATH=/usr/local/opt/qt/ -DQT5_BUILD=true -DWEB_ENGINE=true
    make || exit 1
fi
