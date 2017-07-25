#! /bin/bash

if [ "$QTTYPE" = "4" ]; then
	brew install qt4
fi

if [ "$QTTYPE" = "5" ]; then
	brew install qt
fi

