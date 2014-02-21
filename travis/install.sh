#! /bin/bash

if [ "$QTTYPE" = "4" ]; then
	sudo apt-get install -y libqt4-webkit libqtwebkit-dev qt4-qmake qt4-dev-tools
fi

if [ "$QTTYPE" = "5" ]; then
	sudo apt-get install -y ubuntu-sdk qtquick1-5-dev
fi
