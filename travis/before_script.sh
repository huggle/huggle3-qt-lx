#! /bin/bash

g++ --version

cd /home/travis/build/huggle/huggle3-qt-lx/travis
gunzip *.gz
cd /home/travis/build/huggle/huggle3-qt-lx/huggle

if [ "$QTTYPE" = "4" ]; then
	./configure --qt4 --extension
        cp definitions_prod.hpp definitions.hpp
        cd huggle_release
	make || exit 1
        cd ..
	cd tests/test
	cmake .
	make || exit 1
fi

if [ "$QTTYPE" = "5" ]; then
	./configure --qt5 --extension
        cp definitions_prod.hpp definitions.hpp
        cd huggle_release
	make || exit 1
        cd ..
	cd tests/test
        cmake . -DQT5_BUILD=true
	make || exit 1
fi
