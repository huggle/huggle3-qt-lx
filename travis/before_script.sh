#! /bin/bash

cd /home/travis/build/huggle/huggle3-qt-lx/huggle

if [ "$QTTYPE" = "4" ]; then
	./configure --qt4
	make
	cd tests/test
	qmake
	make
fi

if [ "$QTTYPE" = "5" ]; then
	./configure --qt5 --qmake /usr/lib/x86_64-linux-gnu/qt5/bin/qmake
	make
	cd tests/test
	/usr/lib/x86_64-linux-gnu/qt5/bin/qmake
	make
fi
