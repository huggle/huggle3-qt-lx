#!/bin/sh

echo "Checking all required packages..."

if [ "`which qmake`" = "" ];then
	echo "qt4-qmake is not installed!"
	exit 1
fi

sh update.sh

if [ ! -f version.txt ];then
	echo "Error! unable to create a version file!"
	exit 1
fi

qmake || exit 1
make

echo "Everything was built, you can start huggle by typing"
echo "./huggle"
