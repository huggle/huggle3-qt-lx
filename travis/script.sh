#! /bin/bash

cd /home/travis/build/huggle/huggle3-qt-lx/huggle/tests/test

echo Testing QTTYPE $QTTYPE

test -f tst_testmain || exit 0
./tst_testmain
